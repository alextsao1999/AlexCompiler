//
// Created by Alex on 2022/5/9.
//

#ifndef DRAGON_RISCVEMIT_H
#define DRAGON_RISCVEMIT_H

#include "MachinePass.h"

class RISCVEmit : public MachineBlockPass {
public:
    void runOnFunction(Function &function) override {
        MachineBlockPass::runOnFunction(function);
    }

};


#endif //DRAGON_RISCVEMIT_H
