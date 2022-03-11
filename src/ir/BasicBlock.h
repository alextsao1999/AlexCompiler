//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_BASICBLOCK_H
#define DRAGONIR_BASICBLOCK_H

#include <set>
#include <Node.h>
#include <Instruction.h>
class Function;

class BasicBlock : public NodeWithParent<BasicBlock, Function>, public NodeParent<BasicBlock, Instruction>, public Value {
    friend class Dominance;
public:
    static BasicBlock *Create(Function *parent, std::string_view name);
public:
    BasicBlock(Function *parent, std::string_view name) : NodeWithParent(parent), name(name) {}
    explicit BasicBlock(std::string_view name) : name(name) {}

    const std::string &getName() {
        return name;
    }

    void replace(Instruction *node, Instruction *by) {
        assert(node->getOpcode() != OpcodePhi && by->getOpcode()!= OpcodePhi);
        if (node->isTerminator()) {
            terminator = nullptr;
        }
        if (by->isTerminator()) {
            terminator = by;
        }
        eraseInstr(node);
        NodeParent::replace(node, by);
        addInstr(by);
    }

    void erase(Instruction *node) {
        if (node->isTerminator()) {
            terminator = nullptr;
        }
        eraseInstr(node);
        auto I = NodeParent::erase(node);
        if (node == firstPhi) {
            firstPhi = I.getPointer();
        }
        if (node == lastPhi) {
            lastPhi = I.getPointer();
        }

    }

    void append(Instruction *node) {
        assert(!getTerminator());
        NodeParent::append(node);
        if (node->isTerminator()) {
            terminator = node;
        }
        addInstr(node);
    }

    void insertAfter(Instruction *node, Instruction *after) {
        assert(!node->isTerminator());
        assert(!(node->getOpcode() == OpcodePhi && node != lastPhi));
        NodeParent::insertAfter(node, after);
        addInstr(after);
    }

    void insertBefore(Instruction *node, Instruction *before) {
        assert(!before->isTerminator());
        assert(!(node->getOpcode() == OpcodePhi && before->getOpcode() != OpcodePhi))
        NodeParent::insertBefore(node, before);
        addInstr(before);
    }

    auto begin() {
        return getSubList().begin();
    }

    auto end() {
        return getSubList().end();
    }

    auto getPhis() {
        return iter(NodeListTy::iterator(firstPhi), NodeListTy::iterator(lastPhi));
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

    inline auto &getDominanceFrontier() {
        return dominanceFrontier;
    }

    void dump(std::ostream &os, int level = 0) override {
        os << name << ":    " ;

        DUMP_OS(os << "Doms=(", doms, V, {
            V->dumpAsOperand(os);
        }) << ")  ";

        DUMP_OS(os << "DF=(", getDominanceFrontier(), V, {
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
    inline void addDF(BasicBlock *bb) {
        dominanceFrontier.insert(bb);
    }
    inline void addInstr(Instruction *instr) {
        switch (instr->getOpcode()) {
            case OpcodeBr: {
                auto *Br = static_cast<BranchInst *>(instr);
                successors.insert(Br->getTarget());
                Br->getTarget()->predecessors.insert(this);
                break;
            }
            case OpcodeCondBr: {
                auto *Br = static_cast<CondBrInst *>(instr);
                successors.insert(Br->getTrueTarget());
                successors.insert(Br->getFalseTarget());
                Br->getTrueTarget()->predecessors.insert(this);
                Br->getFalseTarget()->predecessors.insert(this);
                break;
            }
            default:
                break;
        }
    }
    inline void eraseInstr(Instruction *instr) {
        switch (instr->getOpcode()) {
            case OpcodeBr: {
                auto *Br = static_cast<BranchInst *>(instr);
                successors.erase(Br->getTarget());
                Br->getTarget()->predecessors.erase(this);
                break;
            }
            case OpcodeCondBr: {
                auto *Br = static_cast<CondBrInst *>(instr);
                successors.erase(Br->getTrueTarget());
                successors.erase(Br->getFalseTarget());
                Br->getTrueTarget()->predecessors.erase(this);
                Br->getFalseTarget()->predecessors.erase(this);
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
    std::set<BasicBlock *> dominanceFrontier;
    std::set<BasicBlock *> doms;
    BasicBlock *dominator = nullptr;
    Instruction *terminator = nullptr;
    Instruction *firstPhi = nullptr;
    Instruction *lastPhi = nullptr;

};


#endif //DRAGONIR_BASICBLOCK_H
