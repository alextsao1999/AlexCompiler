//
// Created by Alex on 2022/3/15.
//

#ifndef DRAGON_BRANCHELIM_H
#define DRAGON_BRANCHELIM_H

#include "PassManager.h"
#include "Function.h"
#include <algorithm>
class BranchElim : public FunctionPass {
public:
    BranchElim() {}

    void runOnFunction(Function *f) override {
        for (auto &BB : *f) {
            if (auto *Inst = BB.getTerminator()) {
                if (Inst->getOpcode() == OpcodeCondBr) {
                    auto *CondBr = Inst->cast<CondBrInst>();
                    auto *TrueBB = CondBr->getTrueTarget();
                    auto *FalseBB = CondBr->getFalseTarget();
                    assert(TrueBB && FalseBB);
                    if (auto *Val = CondBr->getCond()->as<IntConstant>()) {
                        auto *NewInst = new BranchInst(
                                Val->getVal() == 0 ? CondBr->getFalseTarget() : CondBr->getTrueTarget());
                        Inst->replaceBy(NewInst);
                        continue;
                    }
                    if (TrueBB == FalseBB) {
                        Inst->replaceBy(new BranchInst(TrueBB));
                        continue;
                    }
                }
            }
        }
        for (auto &BB : *f) {
            if (auto *Inst = BB.getTerminator()) {
                while (Inst->getOpcode() == OpcodeBr) {
                    auto *BBSource = Inst->getParent();
                    auto *BBTarget = Inst->getOperand(0)->cast<BasicBlock>();
                    if (!std::all_of(BBTarget->preds_begin(), BBTarget->preds_end(),
                                     [&](BasicBlock *pred) { return pred == BBSource; })) {
                        break;
                    }

                    // exist phi node means the BB has two more different predecessors
                    if (BBTarget->hasPhi()) {
                        break;
                    }

                    // fuse the following blocks
                    auto *First = BBTarget->begin().getPointer();
                    auto *Last = BBTarget->getTerminator();
                    if (First != Last) {
                        BBTarget->getSubList().extract(First, Last);

                        auto &SourceList = BBSource->getSubList();
                        SourceList.inject_before(SourceList.end(), First);
                    }

                    Inst->eraseFromParent();
                    Inst = BBTarget->getTerminator();
                    BBSource->append(Inst);

                    BBTarget->replaceAllUsesWith(BBSource);
                    BBTarget->eraseFromParent();
                }
            }
        }

    }

};


#endif //DRAGON_BRANCHELIM_H
