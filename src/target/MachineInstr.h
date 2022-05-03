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
        Reg,
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
    std::list<Operand> operands;
};

#endif //DRAGON_MACHINEINSTR_H
