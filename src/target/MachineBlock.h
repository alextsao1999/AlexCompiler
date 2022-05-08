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
    std::set<Register> liveInSet;
    std::set<Register> liveOutSet;
    PatternNode *rootNode = nullptr;
public:
    MachineBlock(const std::string &name, BasicBlock *origin) : name(name), origin(origin) {}

    const std::string &getName() const {
        return name;
    }
    ///< Origin of the label
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
    MachineBlock *block;
    MachineInstr *instr;
    MIBuilder() : block(nullptr), instr(new MachineInstr()) {}
    MIBuilder(MachineBlock &block) : block(&block), instr(new MachineInstr()) {}
    ~MIBuilder() {
        if (block) {
            block->append(instr);
        }
    }

    MIBuilder &setOpcode(unsigned opcode) {
        instr->opcode = opcode;
        return *this;
    }

    MIBuilder &addDef(RegID reg) {
        instr->def = Operand::reg(reg);
        return *this;
    }
    MIBuilder &addDef(Operand op) {
        instr->def = op;
        return *this;
    }

    MIBuilder &addOp(Operand op) {
        instr->addOp(op);
        return *this;
    }

    MIBuilder &addUse(RegID reg) {
        addOp(Operand::reg(reg));
        return *this;
    }

    MIBuilder &addImm(int64_t imm) {
        instr->addOp(Operand::imm(imm));
        return *this;
    }

    MachineInstr *build() {
        return instr;
    }

};

#endif //DRAGON_MACHINEBLOCK_H
