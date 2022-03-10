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
public:
    static BasicBlock *Create(Function *parent, std::string_view name);

public:
    BasicBlock(Function *parent, std::string_view name) : NodeWithParent(parent), name(name) {}
    explicit BasicBlock(std::string_view name) : name(name) {}

    const std::string &getName() {
        return name;
    }

    void replace(Instruction *node, Instruction *by) {
        if (node->isTerminator()) {
            terminator = nullptr;
        }
        if (by->isTerminator()) {
            terminator = by;
        }
        NodeParent::replace(node, by);
    }

    void erase(Instruction *node) {
        if (node->isTerminator()) {
            terminator = nullptr;
        }
        NodeParent::erase(node);
    }

    void append(Instruction *node) {
        assert(!getTerminator());
        NodeParent::append(node);
        if (node->isTerminator()) {
            terminator = node;
        }
    }

    void insertAfter(Instruction *node, Instruction *after) {
        assert(!node->isTerminator());
        NodeParent::insertAfter(node, after);
    }

    void insertBefore(Instruction *node, Instruction *before) {
        assert(!before->isTerminator());
        NodeParent::insertBefore(node, before);
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

    void dump(std::ostream &os, int level = 0) override {
        os << name << ": " << std::endl;
        for (auto &Instr: getSubList()) {
            Instr.dump(os);
            os << std::endl;
        }
    }

    void dumpAsOperand(std::ostream &os) override {
        os << "%" << name;
    }

private:
    std::string name;
    std::set<BasicBlock *> predecessors;
    std::set<BasicBlock *> successors;
    BasicBlock *dominator = nullptr;
    Instruction *terminator = nullptr;

};


#endif //DRAGONIR_BASICBLOCK_H
