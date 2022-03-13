//
// Created by Alex on 2022/3/12.
//

#ifndef DRAGON_INLINER_H
#define DRAGON_INLINER_H

#include "PassManager.h"
#include "Function.h"

class Inliner : public FunctionPass {
public:
    void runOnFunction(Function *function) override {
        if (function->isDeclaration()) {
            return;
        }
        std::map<Value *, Value *> ValueMap;

        function->forEach<CallInst>([&](CallInst *inst) {
            Function *Callee = inst->getCallee();
            assert(!Callee->isDeclaration());

            // Map the parameters
            int I = 0;
            for (auto &Param: Callee->getParams()) {
                ValueMap[Param.get()] = inst->getArg(I++);
            }

            AllocaInst *RetVal = nullptr;
            if (auto *RetTy = Callee->getReturnType()) {
                RetVal = new AllocaInst(RetTy);
                inst->insertBeforeThis(RetVal);
            }

            auto *RetBlock = inst->getParent();
            auto *CallSiteBlock = RetBlock->split(inst);

            if (RetVal) {
                auto *RetLoad = new LoadInst(RetVal);
                inst->insertAfterThis(RetLoad);
                inst->replaceAllUsesWith(RetLoad);
            }

            auto CopyBlock = [&](BasicBlock *bb) -> BasicBlock * {
                auto *NewBB = new BasicBlock(bb->getName() + ".inlined");
                for (auto &I : *bb) {
                    switch (I.getOpcode()) {
                        case OpcodeRet: {
                            auto *Ret = I.cast<RetInst>();
                            if (!Ret->isVoidRet()) {
                                assert(RetVal);
                                NewBB->append(new StoreInst(RetVal, Ret->getRetVal()));
                            }
                            NewBB->append(new BranchInst(RetBlock));
                            break;
                        }
                        default: {
                            auto *Cloned = I.clone();
                            ValueMap[&I] = Cloned;
                            NewBB->append(Cloned);
                            break;
                        }
                    }
                }
                return NewBB;
            };

            for (auto &BB: Callee->getSubList()) {
                auto *NewBB = CopyBlock(&BB);
                RetBlock->insertBeforeThis(NewBB);
                ValueMap[&BB] = NewBB;
            }

            auto *Br = CallSiteBlock->getTerminator();
            assert(Br);
            Br->setSuccessor(0, ValueMap[Callee->getEntryBlock()]->cast<BasicBlock>());

            // replace the value of operands
            for (auto &BB: iter(Function::iterator(CallSiteBlock), Function::iterator(RetBlock))) {
                for (auto &Inst : BB) {
                    for (auto &Use: Inst.operands()) {
                        auto *Value = Use.getValue();
                        if (ValueMap.count(Value)) {
                            Use.set(ValueMap[Value]);
                        }
                        if (auto *Phi = Use->as<PhiInst>()) {
                            // replace incoming blocks
                            for (auto K = 0; K < Phi->getOperandNum(); ++K) {
                                if (ValueMap.count(Phi->getIncomingBlock(K))) {
                                    Phi->setIncomingBlock(K, ValueMap[Phi->getIncomingBlock(K)]->cast<BasicBlock>());
                                }
                            }
                        }
                    }
                }
            }

            // remove the call inst
            inst->eraseFromParent();
        });

    }
};


#endif //DRAGON_INLINER_H
