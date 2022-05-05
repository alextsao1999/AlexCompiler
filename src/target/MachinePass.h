//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_MACHINEPASS_H
#define DRAGON_MACHINEPASS_H

#include "Function.h"
#include "PassManager.h"
#include "MachineBlock.h"
class MachinePass : public FunctionPass {
public:
    using FunctionPass::FunctionPass;
};

class MachineBlockPass : public MachinePass {
public:
    void runOnFunction(Function &function) override {
        for (auto &BB: function) {
            auto &MBB = function.mapBlocks[&BB];
            if (MBB == nullptr) {
                MBB = new MachineBlock(BB.dumpOperandToString(), &BB);
                function.blocks.push_back(MBB);
            }
            runOnMachineBlock(*MBB);
        }
    }
    virtual void runOnMachineBlock(MachineBlock &block) = 0;
};

#endif //DRAGON_MACHINEPASS_H
