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
                // create alloca to store return value
                RetVal = new AllocaInst(RetTy);
                inst->insertBeforeThis(RetVal);

                // replace call site with the load of return value
                auto *RetLoad = new LoadInst(RetVal);
                inst->insertAfterThis(RetLoad);
                inst->replaceAllUsesWith(RetLoad);
            }

            auto *RetBlock = inst->getParent(); // return block
            auto *CallSiteBlock = RetBlock->split(inst); // upper block

            // Copy blocks and instructions
            auto CopyBlock = [&](BasicBlock *bb) -> BasicBlock * {
                auto *NewBB = new BasicBlock(bb->getName() + ".inlined");
                for (auto &I : *bb) {
                    if (I.getOpcode() == OpcodeRet) {
                        auto *Ret = I.cast<RetInst>();
                        if (!Ret->isVoidRet()) {
                            assert(RetVal);
                            NewBB->append(new StoreInst(RetVal, Ret->getRetVal()));
                        }
                        NewBB->append(new BranchInst(RetBlock));
                    } else {
                        auto *Cloned = I.clone();
                        ValueMap[&I] = Cloned;
                        NewBB->append(Cloned);
                    }
                }
                return NewBB;
            };
            for (auto &BB: Callee->getSubList()) {
                // Copy every block and insert it after the call site
                auto *NewBB = CopyBlock(&BB);
                RetBlock->insertBeforeThis(NewBB);
                ValueMap[&BB] = NewBB;
            }

            assert(ValueMap.count(Callee->getEntryBlock()));
            auto *InlinedEntry = ValueMap[Callee->getEntryBlock()]->cast<BasicBlock>();

            // jump to the entry block of inlined function
            auto *Br = CallSiteBlock->getTerminator();
            assert(Br && Br->getOpcode() == OpcodeBr);
            // TODO: optimize this
            Br->setSuccessor(0, InlinedEntry);

            // replace the value of operands with the mapped value
            for (auto &BB: iter(Function::iterator(InlinedEntry), Function::iterator(RetBlock))) {
                for (auto &Inst : BB.getSubList()) {
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

            // erase the call inst
            inst->eraseFromParent();
        });

    }
};


#endif //DRAGON_INLINER_H
