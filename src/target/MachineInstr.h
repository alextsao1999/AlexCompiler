//
// Created by Alex on 2022/3/24.
//

#ifndef DRAGON_MACHINEINSTR_H
#define DRAGON_MACHINEINSTR_H

#include <list>
#include "PatternNode.h"

class Operand {
public:
    enum Kind {
        VirReg,
        PhyReg,
        Imm,
    };
    Kind kind;
    Kind getKind() const {
        return kind;
    }
};

class MachineBlock;
class MachineInstr : public NodeWithParent<MachineInstr, MachineBlock> {
public:
    unsigned opcode;
    Operand def;
    std::list<Operand> operands;
    bool hasDef: 1 = true;
    bool hasUse: 1 = true;

    auto defs_begin() {
        return &def;
    }
    auto defs_end() {
        return &def + hasDef;
    }
    auto op_begin() {
        return operands.begin();
    }
    auto op_end() {
        return operands.end();
    }


};

#endif //DRAGON_MACHINEINSTR_H
