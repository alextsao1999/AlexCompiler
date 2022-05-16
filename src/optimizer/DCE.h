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
        std::vector<Instruction *> Worklist;
        function.forEach([&](Instruction *instruction) {
            switch (instruction->getOpcode()) {
                case OpcodeAlloca:
                case OpcodeCast:
                case OpcodeCopy:
                case OpcodePhi:
                case OpcodeNeg:
                case OpcodeNot:
                case OpcodeBinary:
                case OpcodeGetPtr:
                case OpcodeLoad:
                    Worklist.push_back(instruction);
                    break;
                default:
                    break;
            }
        });

        while (!Worklist.empty()) {
            auto *Inst = Worklist.back();
            Worklist.pop_back();
            if (Inst->isNotUsed()) {
                for (auto *Op: Inst->ops()) {
                    if (auto *User = Op->as<Instruction>()) {
                        Worklist.push_back(User);
                    }
                }
                Inst->eraseFromParent();
            }
        }
    }
};

#endif //DRAGON_DCE_H
