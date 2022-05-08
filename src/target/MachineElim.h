//
// Created by Alex on 2022/5/8.
//

#ifndef DRAGON_MACHINEELIM_H
#define DRAGON_MACHINEELIM_H

#include "MachinePass.h"
class MachineElim : public MachineBlockPass {
public:
    void runOnMachineBlock(MachineBlock &block) override {
        auto *F = block.getFunction();
        auto *TI = F->getTargetInfo();
        assert(F && TI);
        for (auto Iter = block.begin(); Iter != block.end();) {
            auto &Inst = *Iter;
            if (TI->isEliminableMove(Inst)) {
                Iter = block.getSubList().erase(Iter);
            } else {
                ++Iter;
            }
        }
    }
};

#endif //DRAGON_MACHINEELIM_H
