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
        //isolatePhiByCopy(function);
        addAssignForPhi(function);
    }

    void isolatePhiByCopy(Function *function) {
        for (auto &BB: *function) {
            for (auto &Inst: BB.getPhis()) {
                auto *Phi = Inst.cast<PhiInst>();
                for (auto &Use: Inst.operands()) {
                    auto *Value = Use.getValue();
                    auto *IncomingBB = Phi->getIncomingBlock(Use);
                    assert(IncomingBB);
                    auto *CopyedValue = new CopyInst(Value);
                    IncomingBB->append(CopyedValue);
                    Use.set(CopyedValue);
                }
            }
        }
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
                    /*auto *Terminator = Pred->getTerminator();
                    assert(Terminator);
                    // update successor
                    // FIXME: Phi?
                    for (int I = 0; I < Terminator->getNumSuccessors(); ++I) {
                        if (Terminator->getSuccessor(I) == &BB) {
                            Terminator->setSuccessor(I, NewBB);
                        }
                    }*/
                    updateBasicBlock(&BB, Pred, NewBB);
                    NewBB->append(new BranchInst(&BB));
                }
            }
        }
    }
    void updateBasicBlock(BasicBlock *bb, BasicBlock *oldBB, BasicBlock *newBB) {
        for (auto Iter = bb->use_begin(); Iter != bb->use_end();) {
            Use &Use = *Iter++;
            if (auto *User = Use.getUser()->as<Instruction>()) {
                if (User->getParent() == oldBB) {
                    if (User->isTerminator()) {
                        Use.set(newBB);
                    }
                }
            }
        }
        for (auto Iter = oldBB->use_begin(); Iter != oldBB->use_end();) {
            Use &Use = *Iter++;
            if (auto *User = Use.getUser()->as<Instruction>()) {
                if (User->getParent() == bb) {
                    if (User->getOpcode() == OpcodePhi) {
                        Use.set(newBB);
                    }
                }
            }
        }
    }
    void addAssignForPhi(Function *function) {
        for (auto &BB: function->getBasicBlockList()) {
            for (auto &Phi: BB.phis()) {
                for (auto &Use: Phi.operands()) {
                    auto *IncomingBB = Phi.getIncomingBlock(Use);
                    auto *Assign = new AssignInst(&Phi, Use.getValue());
                    IncomingBB->append(Assign);
                }
            }
        }
    }

};


#endif //DRAGON_SSADESTRUCTOR_H
