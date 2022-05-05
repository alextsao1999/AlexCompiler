//
// Created by Alex on 2022/3/12.
//

#ifndef DRAGON_CALLGRAPH_H
#define DRAGON_CALLGRAPH_H

#include "PassManager.h"
#include "Function.h"
class CallGraph : public FunctionPass {
public:
    void runOnFunction(Function &function) override {
        function.forEach<CallInst>([&](CallInst *callInst) {
            function.addCallee(callInst->getCallee());
        });
    }

};


#endif //DRAGON_CALLGRAPH_H
