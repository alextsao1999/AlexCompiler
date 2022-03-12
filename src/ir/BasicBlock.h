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
public:
    BasicBlock(Function *parent, std::string_view name) : NodeWithParent(parent), name(name) {}
    explicit BasicBlock(std::string_view name) : name(name) {}

    const std::string &getName() {
        return name;
    }
    Context *getContext();

    void replace(Instruction *node, Instruction *by) {
        assert(node->getOpcode() != OpcodePhi && by->getOpcode() != OpcodePhi);
        eraseInstr(node);
        NodeParent::replace(node, by);
        addInstr(by);
    }

    void erase(Instruction *node) {
        eraseInstr(node);
        NodeParent::erase(node);
    }

    void append(Instruction *node) {
        assert(!getTerminator());
        NodeParent::append(node);
        addInstr(node);
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

    auto begin() {
        return getSubList().begin();
    }

    auto end() {
        return getSubList().end();
    }

    Instruction *getTerminator() {
        return terminator;
    }

    auto &getPredecessors() {
        return predecessors;
    }

    auto &getSuccessors() {
        return successors;
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

    void addPhi(Instruction *phi) {
        assert(phi->getOpcode() == OpcodePhi);
        phi->setParent(this);
        lastPhi = list.insert_after(lastPhi, phi);
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

    void dump(std::ostream &os, int level = 0) override {
        os << name << ":    " ;

        DUMP_OS(os << "Doms=(", domChildren, V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_OS(os << "DF=(", getDomFrontier(), V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_OS(os << "Pred=(", getPredecessors(), V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_OS(os << "Succ=(", getSuccessors(), V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        if (getDominator()) {
            os << "idom = ";
            getDominator()->dumpAsOperand(os);
        }

        os << std::endl;

        for (auto &Instr: getSubList()) {
            Instr.dump(os);
            os << std::endl;
        }

        os << std::endl;

    }
    void dumpAsOperand(std::ostream &os) override {
        os << "%" << name;
    }

private:
    inline void setDominator(BasicBlock *dom) {
        dominator = dom;
    }
    inline void addDomFrontier(BasicBlock *bb) {
        domFrontier.insert(bb);
    }
    inline void addInstr(Instruction *instr) {
        switch (instr->getOpcode()) {
            case OpcodePhi:
                lastPhi = instr;
                break;
            case OpcodeRet:
                terminator = instr;
                break;
            case OpcodeBr: {
                auto *Br = static_cast<BranchInst *>(instr);
                successors.insert(Br->getTarget());
                Br->getTarget()->predecessors.insert(this);
                terminator = instr;
                break;
            }
            case OpcodeCondBr: {
                auto *Br = static_cast<CondBrInst *>(instr);
                successors.insert(Br->getTrueTarget());
                successors.insert(Br->getFalseTarget());
                Br->getTrueTarget()->predecessors.insert(this);
                Br->getFalseTarget()->predecessors.insert(this);
                terminator = instr;
                break;
            }
            default:
                break;
        }
    }
    inline void eraseInstr(Instruction *instr) {
        assert(instr);
        switch (instr->getOpcode()) {
            case OpcodePhi:
                if (lastPhi == instr)
                    lastPhi--;
                break;
            case OpcodeRet:
                terminator = nullptr;
                break;
            case OpcodeBr: {
                auto *Br = static_cast<BranchInst *>(instr);
                successors.erase(Br->getTarget());
                Br->getTarget()->predecessors.erase(this);
                terminator = nullptr;
                break;
            }
            case OpcodeCondBr: {
                auto *Br = static_cast<CondBrInst *>(instr);
                successors.erase(Br->getTrueTarget());
                successors.erase(Br->getFalseTarget());
                Br->getTrueTarget()->predecessors.erase(this);
                Br->getFalseTarget()->predecessors.erase(this);
                terminator = nullptr;
                break;
            }
            default:
                break;
        }
    }

private:
    std::string name;
    std::set<BasicBlock *> predecessors;
    std::set<BasicBlock *> successors;
    std::set<BasicBlock *> domFrontier;
    std::set<BasicBlock *> domChildren;
    BasicBlock *dominator = nullptr;
    Instruction *terminator = nullptr;
    iterator lastPhi = list.end();

};


#endif //DRAGONIR_BASICBLOCK_H
