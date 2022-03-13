//
// Created by Alex on 2022/3/13.
//

#ifndef DRAGON_SSADESTRUCTOR_H
#define DRAGON_SSADESTRUCTOR_H

#include "PassManager.h"
#include "Function.h"
class SSADestructor : public FunctionPass {
public:
    void runOnFunction(Function *function) override {
        splitCriticalEdge(function);
    }

    void splitCriticalEdge(Function *function) {
        std::vector<BasicBlock *> Worklist;
        for (auto &BB: function->getBasicBlockList()) {
            if (BB.hasMultiplePredecessor()) {
                Worklist.insert(Worklist.end(), BB.preds_begin(), BB.preds_end());
            }
            while (!Worklist.empty()) {
                BasicBlock *Pred = Worklist.back();
                Worklist.pop_back();
                if (Pred->hasMultipleSuccessors()) {
                    // Pred -> BB is a critical edge
                    auto *NewBB = new BasicBlock("split.critial.edge");
                    Pred->insertAfterThis(NewBB);
                    auto *Terminator = Pred->getTerminator();
                    assert(Terminator);
                    // update successor
                    // FIXME: Phi?
                    for (int I = 0; I < Terminator->getNumSuccessors(); ++I) {
                        if (Terminator->getSuccessor(I) == &BB) {
                            Terminator->setSuccessor(I, NewBB);
                        }
                    }
                    NewBB->append(new BranchInst(&BB));
                }
            }
        }
    }

};


#endif //DRAGON_SSADESTRUCTOR_H
