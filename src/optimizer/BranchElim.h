//
// Created by Alex on 2022/3/15.
//

#ifndef DRAGON_BRANCHELIM_H
#define DRAGON_BRANCHELIM_H

#include "Common.h"
#include "PassManager.h"
#include "Function.h"
#include "Dominance.h"
#include <algorithm>
class BranchElim : public FunctionPass {
public:
    /// After Branch elimination, the basicblock may be erased.
    /// So we need to update the basicblock's dominator and dom tree children.
    /// Otherwise, the program will crash.
    BranchElim() {}

    void runOnFunction(Function &f) override {
        std::vector<BasicBlock *> Worklist;

        f.forEachBlock([&](BasicBlock *bb) {
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
            /// we comment it out for now.
            // Fixme: Maybe it is not right to eliminate the branch.
            /*if (bb->hasOnlyTerminator() && bb != f->getEntryBlock()) {
                // The basic block only contains one terminator instruction,
                // and it is not the entry block. So this basic block is useless.
                // We can remove it.
                if (auto *Inst = bb->getTerminator()) {
                    if (Inst->getNumSuccessors() == 1) {
                        auto *Succ = Inst->getSuccessor(0);
                        bb->replaceAllUsesWith(Succ);
                        bb->eraseFromParent();
                        return;
                    }
                }
            }*/

            if (bb->isUnreachable()) {
                Worklist.push_back(bb);
            }

        });

        std::set<BasicBlock *> Unreachable;
        while (!Worklist.empty()) {
            auto *BB = Worklist.back();
            Worklist.pop_back();
            if (Unreachable.count(BB)) {
                continue;
            }
            if (BB->isUnreachable()) {
                Unreachable.insert(BB);
                Worklist.insert(Worklist.end(), BB->succs_begin(),
                                BB->succs_end());
            } else {
                if (std::all_of(BB->preds_begin(), BB->preds_end(),
                                [&](BasicBlock *pred) { return Unreachable.count(pred) != 0; })) {
                    Unreachable.insert(BB);
                    Worklist.insert(Worklist.end(), BB->succs_begin(),
                                    BB->succs_end());
                }
            }
        }
        for (auto *BB : Unreachable) {
            for (auto *Succ: BB->succs()) {
                for (auto &V : Succ->getPhis()) {
                    auto *Phi = V.cast<PhiInst>();
                    if (auto *Use = Phi->findIncomingUse(BB)) {
                        Phi->removeIncoming(Use);
                    }
                }
            }
            BB->eraseFromParent();
        }
        Unreachable.clear();

        ///< Combine the redundant basic blocks.
        for (auto &BB : f) {
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
                        // move all instructions from BBTarget to BBSource
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
