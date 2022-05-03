//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_MACHINEPASS_H
#define DRAGON_MACHINEPASS_H

#include "PassManager.h"
#include "MachineBlock.h"
class MachinePass : public FunctionPass {
public:
    void runOnFunction(Function *function) override {

    }
};

class MachineBlockPass : public MachinePass {
public:
    void runOnFunction(Function *function) override {

    }
    virtual void runOnMachineBlock(MachineBlock *block) = 0;
};


#endif //DRAGON_MACHINEPASS_H
