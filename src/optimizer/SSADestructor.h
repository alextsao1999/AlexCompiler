//
// Created by Alex on 2022/3/13.
//

#ifndef DRAGON_SSADESTRUCTOR_H
#define DRAGON_SSADESTRUCTOR_H

#include "PassManager.h"
#include "Function.h"
class SSADestructor : public FunctionPass {
public:
    void runOnFunction(Function &function) override {
        //splitCriticalEdge(&function);
        //isolatePhiByCopy(&function);
        //splitCriticalEdge(&function);
        //addAssignForPhi(&function);
        addCopyAtEnd(&function);
        std::cout << "SSADestructor" << std::endl;
    }

    void isolatePhiByCopy(Function *function) {
        for (auto &BB: *function) {
            for (auto &Phi: BB.phis()) {
                for (auto &Use: Phi.operands()) {
                    auto *Value = Use.getValue();
                    auto *IncomingBB = Phi.getIncomingBlock(Use);
                    auto *CopyedValue = new CopyInst(Value);
                    IncomingBB->append(CopyedValue);
                    CopyedValue->setName(Phi.getName() + ".copy");
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

    // Sequentialize the parallel copy, we have already isolate the copy, so we don't need to do this.
    std::map<BasicBlock *, std::vector<std::pair<Value *, Value *>>> copyAtBegin;
    std::map<BasicBlock *, std::vector<std::pair<Value *, Value *>>> copyAtEnd;
    void addCopyAtEnd(Function *function) {
        for (auto &BB: *function) {
            for (auto &Phi: BB.phis()) {
                for (auto &Use: Phi.operands()) {
                    auto *IncomingBB = Phi.getIncomingBlock(Use);
                    auto &CopyAtEnd = copyAtEnd[IncomingBB];
                    CopyAtEnd.emplace_back(&Phi, Use.getValue());
                }
            }
        }
        for (auto &BB: *function) {
            sequentializeCopyAtEnd(&BB);
        }
    }
    void sequentializeCopyAtEnd(BasicBlock *block) {
        auto &Copies = copyAtEnd[block];
        std::map<Value *, Value *> loc;
        std::map<Value *, Value *> pred;
        std::vector<Value *> todo;
        std::vector<Value *> ready;
        for (auto &[Phi, Value]: Copies) {
            loc[Value] = Value;
            pred[Phi] = Value;
            todo.push_back(Phi);
        }

        for (auto &Left: todo) {
            if (loc[Left] == nullptr) { // 如果copy.Left没有记录最后的储存位置说明不存在环
                ready.push_back(Left);
            }
        }

        while (!todo.empty()) {
            while (!ready.empty()) {
                Value *b = ready.back();
                Value *a = pred[b];
                Value *c = loc[a];
                ready.pop_back();
                // emit c -> b
                loc[a] = b;
                block->append(new AssignInst(b, c));
                if (a == c && pred[a]) {
                    ready.push_back(a);
                }
            }
            Value *b = todo.back();
            todo.pop_back();
            if (b != loc[pred[b]]) {
                // 如果b的初始值的最后储存位置和
                // emit b -> n
                auto *Copy = new CopyInst(b);
                block->append(Copy);
                loc[b] = Copy;
                ready.push_back(b);
            }
        }
    }

};

#endif //DRAGON_SSADESTRUCTOR_H
