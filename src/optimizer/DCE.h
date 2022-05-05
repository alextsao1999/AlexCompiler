//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_DCE_H
#define DRAGON_DCE_H

#include "PassManager.h"
#include "Function.h"

class DCE : public FunctionPass {
public:
    void runOnFunction(Function &function) override {
        function.forEach([&](Instruction *instruction) {
            switch (instruction->getOpcode()) {
                case OpcodeAlloca:
                case OpcodeCast:
                case OpcodeCopy:
                case OpcodePhi:
                case OpcodeUnary:
                case OpcodeBinary:
                case OpcodeGetPtr:
                case OpcodeLoad:
                    if (instruction->isNotUsed()) {
                        instruction->eraseFromParent();
                    }
                    break;
                default:
                    break;
            }
        });
    }
};

#endif //DRAGON_DCE_H
