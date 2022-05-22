//
// Created by Alex on 2022/3/15.
//

#ifndef DRAGON_CONSTANTFOLDER_H
#define DRAGON_CONSTANTFOLDER_H

#include "Instruction.h"
#include "Context.h"

class ConstantFolder {
    Value *lhs;
    Value *rhs;
    BinaryOp op;
public:
    inline static bool canFoldBin(Value *l, Value *r) {
        return l->as<Constant>() && r->as<Constant>();
    }
    inline static Value *foldBin(BinaryOp op, Value *lhs, Value *rhs) {
        auto *LType = lhs->getType();
        auto *RType = rhs->getType();
        assert(LType && RType);
        auto *Context = LType->getContext();
        if (LType->isIntegerType() && RType->isIntegerType()) {
            auto *LHS = lhs->as<IntConstant>();
            auto *RHS = rhs->as<IntConstant>();
            assert(LHS && RHS);
            switch (op) {
                case BinaryOp::Add:
                    return Context->getInt(LHS->getVal() + RHS->getVal());
                case BinaryOp::Sub:
                    return Context->getInt(LHS->getVal() - RHS->getVal());
                case BinaryOp::Mul:
                    return Context->getInt(LHS->getVal() * RHS->getVal());
                case BinaryOp::Div:
                    return Context->getInt(LHS->getVal() / RHS->getVal());
                case BinaryOp::Mod:
                    return Context->getInt(LHS->getVal() % RHS->getVal());
                case BinaryOp::Shl:
                    return Context->getInt(LHS->getVal() << RHS->getVal());
                case BinaryOp::Shr:
                    return Context->getInt(LHS->getVal() >> RHS->getVal());
                case BinaryOp::And:
                    return Context->getInt(LHS->getVal() & RHS->getVal());
                case BinaryOp::Or:
                    return Context->getInt(LHS->getVal() | RHS->getVal());
                case BinaryOp::Xor:
                    return Context->getInt(LHS->getVal() ^ RHS->getVal());
                case BinaryOp::Eq:
                    return Context->getInt(LHS->getVal() == RHS->getVal());
                case BinaryOp::Ne:
                    return Context->getInt(LHS->getVal() != RHS->getVal());
                case BinaryOp::Lt:
                    return Context->getInt(LHS->getVal() < RHS->getVal());
                case BinaryOp::Le:
                    return Context->getInt(LHS->getVal() <= RHS->getVal());
                case BinaryOp::Gt:
                    return Context->getInt(LHS->getVal() > RHS->getVal());
                case BinaryOp::Ge:
                    return Context->getInt(LHS->getVal() >= RHS->getVal());
                default:
                    assert(false);
            }
        }
        return nullptr;
    }
    ConstantFolder(BinaryInst *inst) : lhs(inst->getLHS()), rhs(inst->getRHS()), op(inst->getOp()) {}
    ConstantFolder(BinaryOp op, Value *lhs, Value *rhs) : op(op), lhs(lhs), rhs(rhs) {}
    bool canFold() const {
        return canFoldBin(lhs, rhs);
    }
    Value *fold() {
        return foldBin(op, lhs, rhs);
    }
};


#endif //DRAGON_CONSTANTFOLDER_H
