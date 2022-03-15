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
    Context *context;
public:
    ConstantFolder(BinaryInst *inst) : lhs(inst->getLHS()), rhs(inst->getRHS()), op(inst->getOp()), context(inst->getType()->getContext()) {}
    bool canFold() const {
        return lhs->as<Constant>() && rhs->as<Constant>();
    }
    Value *fold() {
        auto *LType = lhs->getType();
        auto *RType = rhs->getType();
        assert(LType && RType);
        if (LType->isIntegerType() && RType->isIntegerType()) {
            auto *LHS = lhs->as<IntConstant>();
            auto *RHS = rhs->as<IntConstant>();
            assert(LHS && RHS);
            switch (op) {
                case BinaryOp::Add:
                    return context->getInt(LHS->getVal() + RHS->getVal());
                case BinaryOp::Sub:
                    return context->getInt(LHS->getVal() - RHS->getVal());
                case BinaryOp::Mul:
                    return context->getInt(LHS->getVal() * RHS->getVal());
                case BinaryOp::Div:
                    return context->getInt(LHS->getVal() / RHS->getVal());
                case BinaryOp::Mod:
                    return context->getInt(LHS->getVal() % RHS->getVal());
                case BinaryOp::Shl:
                    return context->getInt(LHS->getVal() << RHS->getVal());
                case BinaryOp::Shr:
                    return context->getInt(LHS->getVal() >> RHS->getVal());
                case BinaryOp::And:
                    return context->getInt(LHS->getVal() & RHS->getVal());
                case BinaryOp::Or:
                    return context->getInt(LHS->getVal() | RHS->getVal());
                case BinaryOp::Xor:
                    return context->getInt(LHS->getVal() ^ RHS->getVal());
                case BinaryOp::Eq:
                    return context->getInt(LHS->getVal() == RHS->getVal());
                case BinaryOp::Ne:
                    return context->getInt(LHS->getVal() != RHS->getVal());
                case BinaryOp::Lt:
                    return context->getInt(LHS->getVal() < RHS->getVal());
                case BinaryOp::Le:
                    return context->getInt(LHS->getVal() <= RHS->getVal());
                case BinaryOp::Gt:
                    return context->getInt(LHS->getVal() > RHS->getVal());
                case BinaryOp::Ge:
                    return context->getInt(LHS->getVal() >= RHS->getVal());
                default:
                    assert(false);
            }
        }


        return nullptr;
    }

};


#endif //DRAGON_CONSTANTFOLDER_H
