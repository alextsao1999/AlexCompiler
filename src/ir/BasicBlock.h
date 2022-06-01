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
    static BasicBlock *Create(Function *parent, StrView name);
    template<typename BBTy, typename UseIt>
    class PredIter {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = BBTy;
        using difference_type = std::ptrdiff_t;
        using pointer = BBTy *;
        using reference = BBTy *;
    public:
        mutable UseIt iter;
        PredIter() {}
        inline PredIter(const BBTy *bb) : iter(bb->user_begin()) {
            advance();
        }

        inline void advance() const {
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
            ASSERT(!atEnd());
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

        SuccIter(const BasicBlock *bb) : instr(bb->getTerminator()),
                                   idx(bb->getTerminator() ? bb->getTerminator()->getNumSuccessors() : 0) {}
        SuccIter(const BasicBlock *bb, unsigned idx) : instr(bb->getTerminator()), idx(idx) {}

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
            ASSERT(!atEnd());
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
    using range_pred_iter = IterRange<pred_iterator>;
    using range_succ_iter = IterRange<succ_iterator>;
    using range_iter = IterRange<iterator>;
    using iterator_phi = iterator_as<PhiInst>;
    using range_iter_phi = IterRange<iterator_phi>;
public:
    explicit BasicBlock() {}
    explicit BasicBlock(StrView name) : name(name) {}
    explicit BasicBlock(Function *parent, StrView name);

    // FIXME: It's for debug now.
    unsigned count = 0;
    const std::string &getName() const {
        return name;
    }
    void setName(std::string &newName) {
        name = newName;
    }
    SymbolTable *getSymbolTable() const;
    Context *getContext() const;

    void replace(Instruction *node, Instruction *by) {
        ASSERT(node->getOpcode() != OpcodePhi && by->getOpcode() != OpcodePhi);
        removeInstr(node);
        NodeParent::replace(node, by);
        addInstr(by);
    }

    void erase(Instruction *node) {
        ASSERT(!node->getLastUse()); // check if there is a instruction that use this node.
        removeInstr(node);
        NodeParent::erase(node);
    }

    void remove(Instruction *node) {
        removeInstr(node);
        NodeParent::remove(node);
    }

    void append(Instruction *node) {
        if (getTerminator()) {
            ASSERT(!node->isTerminator() && "BasicBlock already has terminator");
            insertBefore(getTerminator(), node);
        } else {
            NodeParent::append(node);
            addInstr(node);
        }
    }

    void insertAfter(Instruction *node, Instruction *after) {
        // FIXME: 这里最好不能插入Phi
        ASSERT(!node->isTerminator());
        ASSERT(!(node->getOpcode() == OpcodePhi && lastPhi != node)); // 只能在最后一个phi节点后插入(非phi节点)
        NodeParent::insertAfter(node, after);
        addInstr(after);
    }

    void insertBefore(Instruction *node, Instruction *before) {
        // FIXME: 这里最好不能插入Phi
        ASSERT(!before->isTerminator()); // 不能插入terminator
        ASSERT(!(node->getOpcode() == OpcodePhi && before->getOpcode() != OpcodePhi)); // phi节点之前不能插入非phi节点
        NodeParent::insertBefore(node, before);
        addInstr(before);
    }

    BasicBlock *split(Instruction *i, std::string newName = "");

    bool hasOnlyTerminator() {
        return getTerminator() && --iterator(getTerminator()) == list.end();
    }

    inline iterator begin() {
        return getSubList().begin();
    }
    inline iterator end() {
        return getSubList().end();
    }

    // DominatorTree and dominance frontiers
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
        ASSERT(phi->getOpcode() == OpcodePhi);
        phi->setParent(this);
        lastPhi = list.insert_after(lastPhi, phi);
    }

    inline bool hasPhi() const {
        return lastPhi != list.end();
    }

    range_iter_phi phis() const {
        if (lastPhi == list.end()) {
            return iter(iterator_phi(list.end().getPointer()), iterator_phi(list.end().getPointer()));
        }
        return iter(iterator_phi(list.begin().getPointer()), ++iterator_phi(lastPhi.getPointer()));
    }
    range_iter getPhis() const {
        if (lastPhi == list.end()) {
            return iter(list.end(), list.end());
        }
        return iter(list.begin(), ++iterator(lastPhi));
    }

    range_iter instrs() {
        return iter(list.begin(), list.end());
    }
    range_iter getInstrs() const {
        if (lastPhi == list.end()) {
            return iter(list.begin(), list.end());
        }
        return iter(++iterator(lastPhi), list.end());
    }

    Instruction *getTerminator() const {
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
        for (; Begin != End; ++Begin) {
            if (++Count >= 2) {
                return true;
            }
        }
        return false;
    }
    bool hasOnlyTwoPreds() {
        auto Begin = pred_iterator(this);
        auto End = pred_iterator();
        unsigned Count = 0;
        for (; Begin != End; ++Begin) {
            if (++Count >= 2) {
                return ++Begin == End;
            }
        }
        return true;
    }

    inline auto preds() {
        return iter(preds_begin(), preds_end());
    }
    inline auto succs() {
        return iter(succs_begin(), succs_end());
    }

    inline pred_iterator preds_begin() const {
        return pred_iterator(this);
    }
    inline pred_iterator preds_end() const {
        return pred_iterator();
    }

    inline succ_iterator succs_begin() const {
        return succ_iterator(this, 0);
    }
    inline succ_iterator succs_end() const {
        return succ_iterator(this);
    }

    // DFS visit the dominator tree and compute the dominator tree level
    void calculateLevel(unsigned init = 0) {
        this->level = init++;
        for (auto &Child: domChildren) {
            Child->calculateLevel(init);
        }
    }
    inline unsigned getLevel() const {
        return level;
    }
    bool dominates(BasicBlock *rhs) const {
        if (rhs == nullptr /*|| getLevel() >= rhs->getLevel()*/) {
            return false;
        }
        if (this == rhs || this == rhs->dominator) {
            return true;
        }
        if (dominator == rhs) {
            return false;
        }
        do {
            rhs = rhs->dominator;
            if (rhs == this) {
                return true;
            }
        } while (rhs);
        return false;
    }
    bool isUnreachable() const;

    // dump the basic block
    void dump(std::ostream &os) override {
        dumpName(os) << ":    ";
        os << "preds=(" << dump_str(preds()) << ") ";
        os << "succs=(" << dump_str(succs()) << ") ";

        // We must update the dominance infor before we dump the instructions
        if (!domChildren.empty())
            os << "doms=(" << dump_str(domChildren) << ") ";
        if (!getDomFrontier().empty())
            os << "df=(" << dump_str(getDomFrontier()) << ") ";
        if (getDominator())
            os << "idom=" << getDominator()->dumpOperandToString();
        os << std::endl;

        /*for (auto &Instr: getSubList()) {
            Instr.dump(os);
            os << std::endl;
        }*/
        os << dump_str(getSubList(), ValueDumper(), "\n");
    }
    void dumpAsOperand(std::ostream &os) override {
        os << "%";
        dumpName(os);
    }
    inline std::ostream &dumpName(std::ostream &os) const {
        os << name;
        if (auto *ST = getSymbolTable()) {
            os << "." << ST->getCount(this, name);
        }
        return os;
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
        ASSERT(instr);
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
    unsigned level = 0;
    std::set<BasicBlock *> domFrontier; ///< the dominance frontier of this label
    std::set<BasicBlock *> domChildren; ///< children of the dominator
    BasicBlock *dominator = nullptr; ///< immediate dominator
    Instruction *terminator = nullptr; ///< the terminator instruction
    iterator lastPhi = list.end(); ///< last phi instruction

};


#endif //DRAGONIR_BASICBLOCK_H
