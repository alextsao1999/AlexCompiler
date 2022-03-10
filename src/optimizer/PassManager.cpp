//
// Created by Alex on 2022/3/10.
//

#include "PassManager.h"
#include "Module.h"

void FunctionPass::run(Module *module) {
    for (auto &Function : *module) {
        runOnFunction(&Function);
    }

}
