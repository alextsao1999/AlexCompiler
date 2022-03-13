//
// Created by Alex on 2022/3/12.
//

#ifndef DRAGON_INLINER_H
#define DRAGON_INLINER_H

#include "PassManager.h"
#include "Function.h"

class BlockCopyer {

};

class Inliner : public FunctionPass {
public:
    void runOnFunction(Function *function) override {
        if (function->isDeclaration()) {
            return;
        }
        function->forEach<CallInst>([&](CallInst *inst) {
            Function *Callee = inst->getCallee();
            assert(!Callee->isDeclaration());

        });

    }
};


#endif //DRAGON_INLINER_H
