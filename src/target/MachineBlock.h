//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_MACHINEBLOCK_H
#define DRAGON_MACHINEBLOCK_H

#include <set>
#include "Node.h"
#include "MachineInstr.h"
#include "PatternDAG.h"
class BasicBlock;

class MachineBlock : public NodeParent<MachineBlock, MachineInstr, ListAllocTrait<MachineInstr>>, public Node<MachineBlock> {
public:
    std::string name;
    BasicBlock *origin;
    std::vector<MachineBlock *> preds;
    std::vector<MachineBlock *> succs;
    std::set<PatternNode *> liveInSet;
    std::set<PatternNode *> liveOutSet;
    PatternNode *rootNode = nullptr;
    MachineBlock(const std::string &name, BasicBlock *origin) : name(name), origin(origin) {}
    BasicBlock *getOrigin() { return origin; }

    auto instrs() { return iter(begin(), end()); }
    iterator begin() { return list.begin(); }
    iterator end() { return list.end(); }

    void setRootNode(PatternNode *node) { this->rootNode = node; }
    PatternNode *getRootNode() { return rootNode; }

    void append(MachineInstr *instr) {
        list.push_back(instr);
    }
};


class MIBuilder {
public:
    MachineBlock &block;
    MachineInstr *instr;
    MIBuilder(MachineBlock &block) : block(block) {
        instr = new MachineInstr();
    }
    ~MIBuilder() {
        block.append(instr);
    }

    MIBuilder &setOpcode(unsigned opcode) {
        instr->opcode = opcode;
        return *this;
    }

    MIBuilder &addReg(unsigned reg) {
        //instr->operands.push_back(0);
        return *this;
    }

    MIBuilder &addImm(int imm) {
        //instr->operands.push_back(1);
        return *this;
    }

};

#endif //DRAGON_MACHINEBLOCK_H
