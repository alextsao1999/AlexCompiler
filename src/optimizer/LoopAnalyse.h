//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_LOOPANALYSE_H
#define DRAGON_LOOPANALYSE_H

#include "PassManager.h"
#include "Function.h"
#include "LoopInfo.h"
class LoopAnalyse : public BasicBlockPass {
public:
    void runOnBasicBlock(BasicBlock *bb) override {

    }
};


#endif //DRAGON_LOOPANALYSE_H
