//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_SSABUILDER_H
#define DRAGONIR_SSABUILDER_H

#include "PassManager.h"
#include "Context.h"
#include "Function.h"
#include "IDFCalculator.h"

struct VarStatus {
    std::vector<Value *> stack;
    unsigned counter = 0;

    void push(Value *var) {
        assert(var);
        stack.push_back(var);
        if (auto *Instr = var->as<Instruction>()) {
            counter++;
        }
    }

    Value *pop() {
        auto *Res = stack.back();
        stack.pop_back();
        return Res;
    }

    Value *top() {
        assert(!stack.empty());
        return stack.back();
    }

    bool empty() {
        return stack.empty();
    }
};
struct PhiStatus {
    Value *allocaFor = nullptr;
    bool useless = true; // 用来标记Phi语句是否被使用
    std::map<BasicBlock *, Value *> incomings; // 标记来边和对应的SSA值

    void setAlloca(Value *a) {
        assert(allocaFor == nullptr);
        allocaFor = a;
    }

    Value *getAlloca() const {
        return allocaFor;
    }

    bool isUseless() const {
        return useless;
    }

    void setUseless(bool u) {
        useless = u;
    }

    void addIncoming(BasicBlock *bb, Value *val) {
        incomings[bb] = val;
    }

    void fill(PhiInst *node) {
        node->fill(incomings);
    }

};

class SSAConstructor : public FunctionPass {
public:
    std::map<Value *, VarStatus> varStatus; // Var state for alloca
    std::map<PhiInst *, PhiStatus> phiStatus; // Phi state for phi inst
    std::vector<PhiInst *> phiStack;

    void runOnFunction(Function *function) override {
        phiStack.clear();
        varStatus.clear();
        phiStatus.clear();
        std::map<AllocaInst *, std::vector<BasicBlock *>> DefBlocks;
        // Find all allocas def blocks
        for (auto &BB: function->getBasicBlockList()) {
            for (auto &I : BB) {
                if (I.getOpcode() == OpcodeStore) {
                    auto *Store = I.as<StoreInst>();
                    auto *Ptr = Store->getPtr();
                    if (auto *Alloca = Ptr->as<AllocaInst>()) {
                        DefBlocks[Alloca].push_back(&BB);
                    }
                }
            }
        }

        // place phi nodes
        placing(DefBlocks);

        // rename phi nodes
        rename(function->getEntryBlock());

        // prune unless phi nodes
        prune(function->getEntryBlock());

        install();
    }

    void placing(const std::map<AllocaInst *, std::vector<BasicBlock *>> &defs) {
        std::vector<BasicBlock *> Worklist;
        std::map<BasicBlock *, Value *> InWorklist;
        for (auto &[Value, Blocks]: defs) {
            for (auto *BB: Blocks) {
                Worklist.push_back(BB);
                InWorklist[BB] = Value;
            }

            while (!Worklist.empty()) {
                auto *BB = Worklist.back();
                Worklist.pop_back();

                for (auto *D: BB->getDomFrontier()) {
                    if (InWorklist[D] != Value) {
                        StrView Name = Value->getName();
                        auto *PhiNode = PhiInst::Create(Value->getAllocatedType(), D, Name);
                        phiStatus[PhiNode].setAlloca(Value);
                        InWorklist[D] = Value;
                        Worklist.push_back(D);
                    }
                }
            }
        }
    }

    void placingByIDF(const std::map<AllocaInst *, std::vector<BasicBlock *>> &defs) {
        std::map<BasicBlock *, Value *> Placed;
        for (auto &[Value, Blocks]: defs) {
            IDFCalculator IDF;
            IDF.calulate(Blocks);
            for (auto &BB: IDF.IDF) {
                if (Placed[BB] != Value) {
                    StrView Name = Value->cast<Instruction>()->getName();
                    auto *PhiNode = PhiInst::Create(Value->getAllocatedType(), BB, Name);
                    phiStatus[PhiNode].setAlloca(Value);
                    Placed[BB] = Value;
                }
            }
        }

    }

    void rename(BasicBlock *bb) {
        // rename phi nodes
        for (auto &Instr: bb->getPhis()) {
            auto *Phi = Instr.cast<PhiInst>();
            if (phiStatus.find(Phi) == phiStatus.end()) {
                continue;
            }
            auto *Alloca = phiStatus[Phi].getAlloca();
            assert(Alloca);
            varStatus[Alloca].push(Phi);
        }

        for (auto &Instr: bb->getInstrs()) {
            for (auto &Op: Instr.operands()) {
                if (auto *Load = Op->as<LoadInst>()) {
                    if (auto *Alloca = Load->getPtr()->as<AllocaInst>()) {
                        auto &VarStatus = varStatus[Alloca];
                        Op.set(VarStatus.empty() ? bb->getContext()->getUndef() : VarStatus.top());
                    }
                }
            }
            if (Instr.getOpcode() == OpcodeStore) {
                auto *Store = Instr.cast<StoreInst>();
                if (auto *Alloca = Store->getPtr()->as<AllocaInst>()) {
                    varStatus[Alloca].push(Store->getVal());
                }
            }
        }

        // insert phi nodes
        for (auto *Succ: bb->succs()) {
            for (auto &Instr: Succ->getPhis()) {
                auto *Phi = Instr.cast<PhiInst>();
                auto Iter = phiStatus.find(Phi);
                if (Iter == phiStatus.end()) {
                    continue;
                }
                auto &PhiState = Iter->second;
                if (auto *Alloca = PhiState.getAlloca()) {
                    auto &VarStatus = varStatus[Alloca];
                    PhiState.addIncoming(bb, VarStatus.empty() ? bb->getContext()->getUndef() : VarStatus.top());
                }
            }
        }

        for (auto &Dom: bb->getDomChildren()) {
            rename(Dom);
        }

        // Pop all versions
        for (auto &Instr: bb->getPhis()) {
            auto *Phi = Instr.cast<PhiInst>();
            if (phiStatus.find(Phi) == phiStatus.end()) {
                continue;
            }
            auto *Alloca = phiStatus[Phi].getAlloca();
            assert(Alloca);
            varStatus[Alloca].pop();
        }

        for (auto &Instr: bb->getInstrs()) {
            if (Instr.getOpcode() == OpcodeStore) {
                auto *Store = Instr.cast<StoreInst>();
                if (auto *Alloca = Store->getPtr()->as<AllocaInst>()) {
                    varStatus[Alloca].pop();
                }
            }
        }

    }

    void prune(BasicBlock *bb) {
        for (auto &Inst : bb->getInstrs()) {
            for (auto &Op : Inst.operands()) {
                if (auto *Var = Op->as<PhiInst>()) {
                    auto Iter = phiStatus.find(Var);
                    if (Iter == phiStatus.end()) {
                        continue;
                    }
                    Iter->second.setUseless(false);
                    phiStack.push_back(Var);
                }
            }
        }

        while (!phiStack.empty()) {
            auto *Phi = phiStack.back();
            phiStack.pop_back();

            for (auto [Block, Value] : phiStatus[Phi].incomings) {
                if (auto *Def = Value->as<PhiInst>()) {
                    if (phiStatus[Def].isUseless()) {
                        phiStatus[Def].setUseless(false);
                        phiStack.push_back(Def);
                    }
                }
            }
        }

        for (BasicBlock *Dom : bb->getDomChildren()) {
            prune(Dom);
        }
    }

    void install() {
        // erase instructions related to alloca
        std::vector<Instruction *> NeedToRemove;
        for (auto &[Alloca, VarState]: varStatus) {
            for (auto &Use: Alloca->getUsersAs<Instruction>()) {
                NeedToRemove.push_back(&Use);
            }
            NeedToRemove.push_back(Alloca->cast<Instruction>());
        }
        for (auto &Inst: NeedToRemove) {
            if (auto *ST = Inst->getSymbolTable()) {
                ST->removeName(Inst);
            }
            Inst->eraseFromParent();
        }

        // install phi nodes
        for (auto &[Value, PhiState] : phiStatus) {
            if (PhiState.isUseless()) {
                Value->eraseFromParent();
                continue;
            }
            auto *Phi = Value->cast<PhiInst>();
            PhiState.fill(Phi);
        }
    }

};


#endif //DRAGONIR_SSABUILDER_H
