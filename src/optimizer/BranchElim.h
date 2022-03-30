//
// Created by Alex on 2022/3/15.
//

#ifndef DRAGON_BRANCHELIM_H
#define DRAGON_BRANCHELIM_H

#include "Common.h"
#include "PassManager.h"
#include "Function.h"
#include <algorithm>
class BranchElim : public FunctionPass {
public:
    BranchElim() {}

    void runOnFunction(Function *f) override {
        f->forEachBlock([&](BasicBlock *bb) {
            if (auto *Inst = bb->getTerminator()) {
                if (Inst->getOpcode() == OpcodeCondBr) {
                    auto *CondBr = Inst->cast<CondBrInst>();
                    auto *TrueBB = CondBr->getTrueTarget();
                    auto *FalseBB = CondBr->getFalseTarget();
                    assert(TrueBB && FalseBB);
                    if (auto *Val = CondBr->getCond()->as<IntConstant>()) {
                        auto *NewInst = new BranchInst(
                                Val->getVal() == 0 ? CondBr->getFalseTarget() : CondBr->getTrueTarget());
                        Inst->replaceBy(NewInst);
                    } else if (TrueBB == FalseBB) {
                        Inst->replaceBy(new BranchInst(TrueBB));
                    }
                }
            }
            /// eliminate useless branch, but it breaks the tidiness of the IR,
            /// so we comment it out for now
            /*if (bb->hasOnlyTerminator()) {
                if (auto *Inst = bb->getTerminator()) {
                    if (Inst->getNumSuccessors() == 1) {
                        auto *Succ = Inst->getSuccessor(0);
                        bb->replaceAllUsesWith(Succ);
                        bb->eraseFromParent();
                    }
                }
            }*/
        });
        for (auto &BB : *f) {
            if (auto *Inst = BB.getTerminator()) {
                while (Inst->getOpcode() == OpcodeBr) {
                    auto *BBSource = &BB;
                    auto *BBTarget = Inst->getOperand(0)->cast<BasicBlock>();
                    if (!std::all_of(BBTarget->preds_begin(), BBTarget->preds_end(),
                                     [&](BasicBlock *pred) { return pred == BBSource; })) {
                        break;
                    }
                    // exist phi node means the BB has two more different predecessors
                    if (BBTarget->hasPhi())
                        break;
                    assert(BBTarget != BBSource);
                    // first erase old terminator
                    Inst->eraseFromParent();
                    // fuse the following blocks
                    assert(BBTarget->getTerminator());
                    auto First = BBTarget->begin();
                    auto Last = BBTarget->end();
                    while (First != Last) {
                        Inst = &*First++;
                        BBSource->append(Inst);
                    }
                    // erase target block
                    BBTarget->replaceAllUsesWith(BBSource);
                    BBTarget->eraseFromParent();
                }
            }
        }

    }

};


#endif //DRAGON_BRANCHELIM_H
