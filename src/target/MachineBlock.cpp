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
    ASSERT(Fun);
    for (auto &Op: instr->defs()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getReg()].insert(&Op);
        }
    }
    for (auto &Op: instr->uses()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getReg()].insert(&Op);
        }
    }
}

void MachineBlock::remove(MachineInstr *instr) {
    auto *Fun = getFunction();
    ASSERT(Fun);
    for (auto &Op: instr->defs()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getReg()].erase(&Op);
        }
    }
    for (auto &Op: instr->uses()) {
        if (Op.isReg()) {
            Fun->mapOperands[Op.getReg()].erase(&Op);
        }
    }
    NodeParent::remove(instr);
}
