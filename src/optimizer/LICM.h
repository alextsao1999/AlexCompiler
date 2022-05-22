//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_LICM_H
#define DRAGON_LICM_H

/**
 * Loop invariant code motion.
 */

#include "PassManager.h"
#include "Function.h"
#include "LoopAnalyse.h"

class LICM : public LoopPass {
public:
    void runOnLoop(Loop &loop) override {
        std::vector<Instruction *> Invariants;
        auto *Preheader = loop.getPreheader();
        for (auto &BB: loop) {
            for (auto &Inst: *BB) {
                if (loop.isLoopInvariant(&Inst)) {
                    Invariants.push_back(&Inst);
                }
            }
        }

        while (!Invariants.empty()) {
            auto *Inst = Invariants.back();
            Invariants.pop_back();
            /*auto *BB = Inst->getParent();
            if (Preheader->getTerminator()->getNumSuccessors() == 1) {
                Preheader->getTerminator()->eraseFromParent();
            }
            Inst->moveBefore(Preheader->getTerminator());
            if (loop.isLoopInvariant(Inst)) {
                Invariants.push_back(Inst);
            }*/
        }
    }
};


#endif //DRAGON_LICM_H
