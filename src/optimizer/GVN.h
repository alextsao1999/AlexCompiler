//
// Created by Alex on 2022/3/14.
//

#ifndef DRAGON_GVN_H
#define DRAGON_GVN_H

#include <unordered_map>
#include <algorithm>
#include "PassManager.h"
#include "Function.h"
#include "Instruction.h"
#include "ConstantFolder.h"
struct VNExpr {
    Value *lhs = nullptr;
    Value *rhs = nullptr;
    BinaryOp op = BinaryOp::None;
    //VNExpr() {}
    //VNExpr(BinaryInst *bin) : lhs(bin->getLHS()), rhs(bin->getRHS()), op(bin->getOp()) {}
    bool operator==(const VNExpr &e) const {
        return (std::tie(lhs, rhs) == std::tie(e.lhs, e.rhs) ||
                (isCommutative(op) && std::tie(lhs, rhs) == std::tie(e.rhs, e.lhs))) && op == e.op;
    }
};

struct VNExprHasher {
    size_t operator()(const VNExpr &e) const {
        return std::hash<void *>()(e.lhs) + std::hash<void *>()(e.rhs) + e.op;
    }
};

template<typename T>
using VNTable = std::unordered_map<VNExpr, T, VNExprHasher>;

class VNScope {
private:
    VNTable<Value *> maps;
    VNScope *outer = nullptr;
public:
    VNScope(VNScope *outer) : outer(outer) {}

    Value *get(BinaryInst *bin) {
        VNExpr Exp{bin->getLHS(), bin->getRHS(), bin->getOp()};
        for (auto *Iter = this; Iter; Iter = Iter->outer) {
            if (Iter->maps.count(Exp)) {
                return Iter->maps[Exp];
            }
        }
        return nullptr;
    }

    void set(BinaryInst *bin, Value *obj) {
        VNExpr Exp{bin->getLHS(), bin->getRHS(), bin->getOp()};
        maps[Exp] = obj;
    }

};

// Value Numbering, Briggs, Cooper, and Simpson, 1997
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.36.8877&rep=rep1&type=pdf
// Rename phis and global value number

class GVN : public FunctionPass {
public:
    std::map<Value *, Value *> mapVN;
    std::vector<Instruction *> needToDelete;

    void runOnFunction(Function *function) override {
        mapVN.clear();
        for (auto &Param : function->getParams()) {
            mapVN[Param.get()] = Param.get();
        }
        doGVN(function->getEntryBlock());
        for (auto &Instr : needToDelete) {
            Instr->eraseFromParent();
        }
        needToDelete.clear();
    }

    void doGVN(BasicBlock *bb, VNScope *outer = nullptr) {
        VNScope Scope(outer);
        for (auto &Phi : bb->getPhis()) {
            if (isMeaningless(&Phi) || isRedundant(bb, &Phi)) {
                assert(Phi.getOperandNum() > 0);
                mapVN[&Phi] = getVN(Phi.getOperand(0));
                needToDelete.push_back(&Phi);
            } else {
                mapVN[&Phi] = &Phi;
            }
        }

        for (auto &Inst : *bb) {
            for (auto &Use : Inst.operands()) {
                if (auto *VN = getVN(Use.getValue())) {
                    Use.set(VN);
                }
            }

            if (auto *BinOp = Inst.as<BinaryInst>()) {
                ConstantFolder Folder(BinOp);
                if (Folder.canFold()) {
                    needToDelete.push_back(BinOp);
                    mapVN[BinOp] = Folder.fold();
                    continue;
                }
                if (auto *Res = Scope.get(BinOp)) {
                    // Scope中之前已经存在相关表达式 x -> y op z 将x的user都替换为之前计算的结果
                    mapVN[BinOp] = Res;
                    needToDelete.push_back(BinOp);
                } else {
                    // 不存在 新建一个VN
                    mapVN[BinOp] = BinOp;
                    Scope.set(BinOp, BinOp);
                }
            }
        }

        for (auto *Succ : bb->succs()) {
            for (auto &Phi: Succ->getPhis()) {
                assert(Phi.getOpcode() == OpcodePhi);
                adjustPhiNode(Succ, Phi.cast<PhiInst>());
            }
        }

        for (auto &Dom : bb->getDomChildren()) {
            doGVN(Dom, &Scope);
        }
    }

    inline Value *getVN(Value *v) {
        return mapVN[v];
    }

    void adjustPhiNode(BasicBlock *bb, PhiInst *phi) {
        if (auto *Use = phi->findIncoming(bb)) {
            if (auto *VN = getVN(Use->getValue())) {
                Use->set(VN);
            }
        }
    }

    bool isMeaningless(Instruction *phi) {
        assert(phi->getOpcode() == OpcodePhi);
        if (phi->getOperandNum() < 2) {
            return true;
        }
        auto *Begin = phi->begin();
        auto *End = phi->end();
        auto *BeginVN = getVN(Begin->getValue());
        if (!BeginVN)
            return false;
        return std::all_of(Begin, End, [&](auto &i) -> bool {
            return BeginVN == getVN(i.getValue());
        });
    }

    bool isRedundant(BasicBlock *bb, Instruction *phi) {
        assert(phi->getOpcode() == OpcodePhi);
        auto *PhiVN = getVN(phi);
        for (auto &Phi: bb->getPhis()) {
            if (phi == &Phi) {
                continue;
            }
            if (PhiVN && getVN(&Phi) == PhiVN) {
                return true;
            }
        }
        return false;
    }



};


#endif //DRAGON_GVN_H
