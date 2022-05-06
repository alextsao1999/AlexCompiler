//
// Created by Alex on 2022/5/3.
//

#include "MachineBlock.h"
#include "Function.h"

Function *MachineBlock::getFunction() {
    return origin ? origin->getParent() : nullptr;
}

void MachineBlock::append(MachineInstr *instr) {
    NodeParent::append(instr);
    auto *Fun = getFunction();
    assert(Fun);
    for (auto &Op: instr->defs()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getOrigin()].insert(&Op);
        }
    }
    for (auto &Op: instr->op()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getOrigin()].insert(&Op);
        }
    }
}

void MachineBlock::remove(MachineInstr *instr) {
    auto *Fun = getFunction();
    assert(Fun);
    for (auto &Op: instr->defs()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getOrigin()].erase(&Op);
        }
    }
    for (auto &Op: instr->op()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getOrigin()].erase(&Op);
        }
    }
    NodeParent::remove(instr);
}
