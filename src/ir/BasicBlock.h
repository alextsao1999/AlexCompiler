//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_BASICBLOCK_H
#define DRAGONIR_BASICBLOCK_H

#include <set>
#include <Node.h>
#include <Instruction.h>
class Function;

class BasicBlock : public Value, public NodeWithParent<BasicBlock, Function>, public NodeParent<BasicBlock, Instruction> {
    friend class Dominance;
public:
    static BasicBlock *Create(Function *parent, std::string_view name);
    template<typename BBTy, typename UseIt>
    class PredIter {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = BBTy;
        using difference_type = std::ptrdiff_t;
        using pointer = BBTy *;
        using reference = BBTy *;
    public:
        UseIt iter;
        PredIter() {}
        inline PredIter(BBTy *bb) : iter(bb->user_begin()) {
            advance();
        }

        inline void advance() {
            while (!iter.atEnd()) {
                if (iter->template as<TerminatorInst>()) {
                    break;
                }
                ++iter;
            }
        }

        inline bool operator==(const PredIter &other) const {
            return iter == other.iter;
        }

        inline bool operator!=(const PredIter &other) const {
            return iter != other.iter;
        }

        inline bool atEnd() const {
            return iter.atEnd();
        }

        inline PredIter &operator++() {
            assert(!atEnd());
            ++iter;
            advance();
            return *this;
        }

        PredIter operator++(int) {
            PredIter Tmp = *this;
            ++(*this);
            return Tmp;
        }

        inline pointer operator->() const {
            return iter->template cast<Instruction>()->getParent();
        }

        inline reference operator*() const {
            return iter->template cast<Instruction>()->getParent();
        }

    };
    class SuccIter {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = BasicBlock;
        using difference_type = std::ptrdiff_t;
        using pointer = BasicBlock *;
        using reference = BasicBlock *;
    public:
        Instruction *instr;
        unsigned idx;

        SuccIter(BasicBlock *bb) : instr(bb->getTerminator()), idx(bb->getTerminator()->getNumSuccessors()) {}
        SuccIter(BasicBlock *bb, unsigned idx) : instr(bb->getTerminator()), idx(idx) {}

        inline bool operator==(const SuccIter &other) const {
            return std::tie(instr, idx) == std::tie(other.instr, other.idx);
        }

        inline bool operator!=(const SuccIter &other) const {
            return std::tie(instr, idx) != std::tie(other.instr, other.idx);
        }

        inline bool atEnd() const {
            return idx >= instr->getNumSuccessors();
        }

        inline void set(BasicBlock *bb) {
            instr->setSuccessor(idx, bb);
        }

        inline SuccIter &operator++() {
            assert(!atEnd());
            ++idx;
            return *this;
        }

        SuccIter operator++(int) {
            SuccIter Tmp = *this;
            ++(*this);
            return Tmp;
        }

        inline pointer operator->() const {
            return instr->getSuccessor(idx);
        }

        inline reference operator*() const {
            return instr->getSuccessor(idx);
        }

    };
    using pred_iterator = PredIter<BasicBlock, Value::UserIterator>;
    using succ_iterator = SuccIter;
public:
    BasicBlock(Function *parent, std::string_view name) : NodeWithParent(parent), name(name) {}
    explicit BasicBlock(std::string_view name) : name(name) {}

    const std::string &getName() {
        return name;
    }
    Context *getContext();

    void replace(Instruction *node, Instruction *by) {
        assert(node->getOpcode() != OpcodePhi && by->getOpcode() != OpcodePhi);
        removeInstr(node);
        NodeParent::replace(node, by);
        addInstr(by);
    }

    void erase(Instruction *node) {
        removeInstr(node);
        NodeParent::erase(node);
    }

    void remove(Instruction *node) {
        removeInstr(node);
        NodeParent::remove(node);
    }

    void append(Instruction *node) {
        if (getTerminator()) {
            assert(!node->isTerminator());
            insertBefore(getTerminator(), node);
        } else {
            NodeParent::append(node);
            addInstr(node);
        }
    }

    void insertAfter(Instruction *node, Instruction *after) {
        // FIXME: 这里最好不能插入Phi
        assert(!node->isTerminator());
        assert(!(node->getOpcode() == OpcodePhi && node != lastPhi)); // 只能在最后一个phi节点后插入(非phi节点)
        NodeParent::insertAfter(node, after);
        addInstr(after);
    }

    void insertBefore(Instruction *node, Instruction *before) {
        // FIXME: 这里最好不能插入Phi
        assert(!before->isTerminator()); // 不能插入terminator
        assert(!(node->getOpcode() == OpcodePhi && before->getOpcode() != OpcodePhi)); // phi节点之前不能插入非phi节点
        NodeParent::insertBefore(node, before);
        addInstr(before);
    }

    BasicBlock *split(Instruction *i, std::string newName = "") {
        if (newName.empty()) {
            newName = getName() + ".split";
        }
        BasicBlock *NewBB = new BasicBlock(newName);
        insertBeforeThis(NewBB);

        auto First = list.begin();
        list.extract(First, i);

        if (First != i) {
            auto &NewBBList = NewBB->getSubList();
            NewBBList.inject_before(NewBBList.end(), First.getPointer());
        }

        replaceAllUsesWith(NewBB);
        NewBB->append(new BranchInst(this));
        return NewBB;
    }

    inline auto begin() {
        return getSubList().begin();
    }

    inline auto end() {
        return getSubList().end();
    }

    inline BasicBlock *getDominator() {
        return dominator;
    }

    inline auto &getDomFrontier() {
        return domFrontier;
    }

    inline auto &getDomChildren() {
        return domChildren;
    }

    inline void addPhi(Instruction *phi) {
        assert(phi->getOpcode() == OpcodePhi);
        phi->setParent(this);
        lastPhi = list.insert_after(lastPhi, phi);
    }

    inline bool hasPhi() const {
        return lastPhi != list.end();
    }

    auto getPhis() {
        if (lastPhi == list.end()) {
            return iter(list.end(), list.end());
        }
        return iter(list.begin(), ++iterator(lastPhi));
    }

    auto getInstrs() {
        if (lastPhi == list.end()) {
            return iter(list.begin(), list.end());
        }
        return iter(++iterator(lastPhi), list.end());
    }

    Instruction *getTerminator() {
        return terminator;
    }

    unsigned getNumSuccessors() {
        return terminator ? terminator->getNumSuccessors() : 0;
    }

    bool hasMultipleSuccessors() {
        return getNumSuccessors() > 1;
    }

    bool hasMultiplePredecessor() {
        auto Begin = pred_iterator(this);
        auto End = pred_iterator();
        unsigned Count = 0;
        for (auto I = Begin; I != End; ++I) {
            if (++Count >= 2) {
                return true;
            }
        }
        return false;
    }

    auto preds() {
        return iter(pred_iterator(this), pred_iterator());
    }

    auto preds_begin() {
        return pred_iterator(this);
    }

    auto preds_end() {
        return pred_iterator();
    }

    auto succs() {
        assert(getTerminator());
        return iter(succ_iterator(this, 0), succ_iterator(this));
    }

    void dump(std::ostream &os) override {
        os << name << ": " ;

        /*DUMP_PTR(os << "preds=(", preds(), V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_PTR(os << "succs=(", succs(), V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_PTR(os << "doms=(", domChildren, V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_PTR(os << "df=(", getDomFrontier(), V, {
            V->dumpAsOperand(os);
        }) << ")";

        if (getDominator()) {
            os << "  idom=";
            getDominator()->dumpAsOperand(os);
        }*/

        os << std::endl;

        for (auto &Instr: getSubList()) {
            Instr.dump(os);
            os << std::endl;
        }

    }
    void dumpAsOperand(std::ostream &os) override {
        os << "%" << name;
    }
private:
    inline void addInstr(Instruction *instr) {
        switch (instr->getOpcode()) {
            case OpcodePhi:
                lastPhi = instr;
                break;
            case OpcodeBr:
            case OpcodeCondBr:
            case OpcodeRet:
                terminator = instr;
                break;
            default:
                break;
        }
    }
    inline void removeInstr(Instruction *instr) {
        assert(instr);
        switch (instr->getOpcode()) {
            case OpcodePhi:
                if (lastPhi == instr)
                    lastPhi--;
                break;
            case OpcodeBr:
            case OpcodeCondBr:
            case OpcodeRet:
                terminator = nullptr;
                break;
            default:
                break;
        }
    }
    inline void setDominator(BasicBlock *dom) {
        dominator = dom;
    }
    inline void addDomFrontier(BasicBlock *bb) {
        domFrontier.insert(bb);
    }
    inline void clearDomInfo() {
        dominator = nullptr;
        domFrontier.clear();
        domChildren.clear();
    }

private:
    std::string name;
    std::set<BasicBlock *> domFrontier;
    std::set<BasicBlock *> domChildren;
    BasicBlock *dominator = nullptr;
    Instruction *terminator = nullptr;
    iterator lastPhi = list.end();

};


#endif //DRAGONIR_BASICBLOCK_H
