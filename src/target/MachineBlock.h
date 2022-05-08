//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_MACHINEBLOCK_H
#define DRAGON_MACHINEBLOCK_H

#include <set>
#include "Node.h"
#include "MachineInstr.h"
class Function;
class BasicBlock;

class MachineBlock : public NodeParent<MachineBlock, MachineInstr, ListAllocTrait<MachineInstr>>, public Node<MachineBlock> {
public:
    unsigned level = 0;
    std::string name;
    BasicBlock *origin;
    std::vector<MachineBlock *> preds;
    std::vector<MachineBlock *> succs;
    std::set<PatternNode *> liveInSet;
    std::set<PatternNode *> liveOutSet;
    PatternNode *rootNode = nullptr;
public:
    MachineBlock(const std::string &name, BasicBlock *origin) : name(name), origin(origin) {}

    const std::string &getName() const {
        return name;
    }
    ///< Origin of the block
    BasicBlock *getOrigin() { return origin; }
    Function *getFunction();

    auto instrs() { return iter(begin(), end()); }
    iterator begin() { return list.begin(); }
    iterator end() { return list.end(); }

    void setRootNode(PatternNode *node) { this->rootNode = node; }
    PatternNode *getRootNode() { return rootNode; }

    void append(MachineInstr *instr);
    void remove(MachineInstr *instr);

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

    MIBuilder &addOp(Operand op) {
        instr->addOp(op);
        return *this;
    }

    MIBuilder &addOp(PatternNode *node) {
        instr->addOp(Operand::from(node));
        return *this;
    }

    MIBuilder &addImm(int64_t imm) {
        instr->addOp(Operand::imm(imm));
        return *this;
    }

};

#endif //DRAGON_MACHINEBLOCK_H
