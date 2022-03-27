//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_LOOPANALYSE_H
#define DRAGON_LOOPANALYSE_H

#include "PassManager.h"
#include "Function.h"
#include "LoopInfo.h"
class LoopAnalyse : public FunctionPass {
public:
    void runOnFunction(Function *function) override {
        for (auto &BB: function->getBasicBlockList()) {
            for (auto *Pred: BB.preds()) {
                if (BB.dominates(Pred)) {
                    discover(function->loops.emplace_back(&BB, Pred));
                }
            }
        }
    }

    std::vector<BasicBlock *> cfgWorklist;
    void discover(Loop &loop) {
        // discover the loop
        cfgWorklist.push_back(loop.getBackedge());
        while (!cfgWorklist.empty()) {
            auto *PredBB = cfgWorklist.back();
            cfgWorklist.pop_back();
            if (loop.addBlock(PredBB)) {
                cfgWorklist.insert(cfgWorklist.end(), PredBB->preds_begin(), PredBB->preds_end());
            }
        }

        // Set the preheader of the loop
        if (loop.getHeader()->hasOnlyTwoPreds()) {
            for (auto *Pred : loop.getHeader()->preds()) {
                if (!loop.contains(Pred)) {
                    loop.setPreheader(Pred);
                }
            }
        }

        // Set outers of the loop
        for (auto &BB: loop.blocks) {
            for (auto *Succ: BB->succs()) {
                if (!loop.contains(Succ)) {
                    loop.outers.insert(Succ);
                }
            }
        }

        // Done
    }

};

class LoopPass : public FunctionPass {
public:
    virtual void runOnLoop(Loop *loop) = 0;
    void runOnFunction(Function *function) override {
        for (auto &Loop: function->loops) {
            runOnLoop(&Loop);
        }
    }
};


#endif //DRAGON_LOOPANALYSE_H
