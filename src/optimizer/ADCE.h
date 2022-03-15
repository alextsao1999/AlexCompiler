//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_ADCE_H
#define DRAGON_ADCE_H

#include "PassManager.h"
#include "Function.h"

class ADCE : public FunctionPass {
public:
    std::set<Instruction *> lives;
    void runOnFunction(Function *function) override {
        lives.clear();
        std::vector<Instruction *> Worklist;
        for (auto &BB: *function) {
            for (auto &I: BB) {
                if (I.hasSideEffects()) {
                    Worklist.push_back(&I);
                }
            }
        }

        while (!Worklist.empty()) {
            auto *I = Worklist.back();
            Worklist.pop_back();
            markLive(I);
            for (auto &Op: I->operands()) {
                if (auto *Inst = Op->as<Instruction>()) {
                    Worklist.push_back(Inst);
                }
            }
        }

        function->forEach([&](Instruction *instr) {
            if (!isLive(instr)) {
                instr->eraseFromParent();
            }
        });

    }

    void markLive(Instruction *i) {
        lives.insert(i);
    }

    bool isLive(Instruction *i) {
        return lives.count(i);
    }

};


#endif //DRAGON_ADCE_H
