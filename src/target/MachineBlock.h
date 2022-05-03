//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_MACHINEBLOCK_H
#define DRAGON_MACHINEBLOCK_H

#include "Node.h"
#include "MachineInstr.h"

class MachineBlock : public NodeParent<MachineBlock, MachineInstr> {
public:
    NodeList<MachineInstr> list;
};

#endif //DRAGON_MACHINEBLOCK_H
