//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_SSABUILDER_H
#define DRAGONIR_SSABUILDER_H

#include "PassManager.h"
#include "Function.h"

class SSABuilder : public FunctionPass {
public:

    void runOnFunction(Function *function) override {
        std::map<Value *, std::vector<Value *>> Defs;
        // Find all allocas
        for (auto &BB: function->getBasicBlockList()) {
            for (auto &I : BB) {
                if (I.getOpcode() == OpcodeStore) {
                    auto *Store = I.as<StoreInst>();
                    auto *Ptr = Store->getPtr();
                    if (Ptr->getOpcode() == OpcodeAlloca) {
                        Defs[Ptr].push_back(Store->getVal());
                    }
                }
            }
        }

    }

};


#endif //DRAGONIR_SSABUILDER_H
