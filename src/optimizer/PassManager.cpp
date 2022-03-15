//
// Created by Alex on 2022/3/10.
//

#include "PassManager.h"
#include "Module.h"

void FunctionPass::run(Module *module) {
    initialize(module);
    for (auto &Function : *module) {
        runOnFunction(&Function);
    }
    finalize(module);
}

void BasicBlockPass::runOnFunction(Function *function) {
    for (auto &BasicBlock: function->getSubList()) {
        runOnBasicBlock(&BasicBlock);
    }
}
