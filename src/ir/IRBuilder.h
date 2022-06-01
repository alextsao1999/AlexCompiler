//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_IRBUILDER_H
#define DRAGONIR_IRBUILDER_H

#include <BasicBlock.h>
#include <Instruction.h>
#include <Function.h>
#include "Module.h"
#include "Context.h"

class IRBuilder {
public:
    IRBuilder() {}
    IRBuilder(BasicBlock *bb) : bb(bb) {}
    IRBuilder(Function *f) : bb(&f->getSubList().back()) {
        ASSERT(!f->getSubList().empty());
    }

    void setInsertPoint(Function *f) {
        ASSERT(!f->getSubList().empty());
        bb = &f->getSubList().back();
    }

    inline BasicBlock *getInsertBlock() const {
        return bb;
    }

    void setInsertPoint(BasicBlock *b) { this->bb = b; }

    template<typename Ty>
    Ty *insert(Ty *instr, StrView name) {
        ASSERT(bb && instr);
        bb->append(instr);
        instr->setName(name);
        return instr;
    }

    template<typename Ty>
    Ty *insert(Ty *instr) {
        ASSERT(bb && instr);
        bb->append(instr);
        return instr;
    }

    Context *getContext() {
        return bb->getParent()->getContext();
    }

    auto *getUndef() {
        return getContext()->getUndef();
    }

    auto *getInt(int64_t val) {
        return getContext()->getInt(val);
    }

    auto *createCondBr(Value *cond, BasicBlock *ifTrue, BasicBlock *ifFalse) {
        return insert(new CondBrInst(cond, ifTrue, ifFalse));
    }

    auto *createBr(BasicBlock *target) {
        return insert(new BranchInst(target));
    }

    auto *createAlloca(Type *ty, StrView name = "") {
        return insert(new AllocaInst(ty), name);
    }

    auto *createBinary(BinaryOp op, Value *lhs, Value *rhs, StrView name = "") {
        Type *Ty = Type::getMaxType(lhs->getType(), rhs->getType());
        return insert(new BinaryInst(Ty, op, lhs, rhs), name);
    }

    auto *createAdd(Value *lhs, Value *rhs, StrView name = "add") {
        return createBinary(BinaryOp::Add, lhs, rhs, name);
    }

    auto *createSub(Value *lhs, Value *rhs, StrView name = "sub") {
        return createBinary(BinaryOp::Sub, lhs, rhs, name);
    }

    auto *createMul(Value *lhs, Value *rhs, StrView name = "mul") {
        return createBinary(BinaryOp::Mul, lhs, rhs, name);
    }

    auto *createDiv(Value *lhs, Value *rhs, StrView name = "div") {
        return createBinary(BinaryOp::Div, lhs, rhs, name);
    }

    auto *createRem(Value *lhs, Value *rhs, StrView name = "rem") {
        return createBinary(BinaryOp::Rem, lhs, rhs, name);
    }

    // create binary
    auto *createAnd(Value *lhs, Value *rhs, StrView name = "and") {
        return createBinary(BinaryOp::And, lhs, rhs, name);
    }

    auto *createOr(Value *lhs, Value *rhs, StrView name = "or") {
        return createBinary(BinaryOp::Or, lhs, rhs, name);
    }

    auto *createXor(Value *lhs, Value *rhs, StrView name = "xor") {
        return createBinary(BinaryOp::Xor, lhs, rhs, name);
    }

    auto *createShl(Value *lhs, Value *rhs, StrView name = "shl") {
        return createBinary(BinaryOp::Shl, lhs, rhs, name);
    }

    auto *createShr(Value *lhs, Value *rhs, StrView name = "shr") {
        return createBinary(BinaryOp::Shr, lhs, rhs, name);
    }

    auto *createEq(Value *lhs, Value *rhs, StrView name = "eq") {
        return createBinary(BinaryOp::Eq, lhs, rhs, name);
    }

    auto *createNe(Value *lhs, Value *rhs, StrView name = "ne") {
        return createBinary(BinaryOp::Ne, lhs, rhs, name);
    }

    auto *createLt(Value *lhs, Value *rhs, StrView name = "lt") {
        return createBinary(BinaryOp::Lt, lhs, rhs, name);
    }

    auto *createLe(Value *lhs, Value *rhs, StrView name = "le") {
        return createBinary(BinaryOp::Le, lhs, rhs, name);
    }

    auto *createGt(Value *lhs, Value *rhs, StrView name = "gt") {
        return createBinary(BinaryOp::Gt, lhs, rhs, name);
    }

    auto *createGe(Value *lhs, Value *rhs, StrView name = "ge") {
        return createBinary(BinaryOp::Ge, lhs, rhs, name);
    }

    auto *createLoad(Value *ptr, StrView name = "load") {
        return insert(new LoadInst(ptr), name);
    }

    auto *createStore(Value *ptr, Value *val) {
        return insert(new StoreInst(ptr, val));
    }

    auto *createCall(Function *func, const std::vector<Value *> &args, StrView name = "call") {
        return insert(new CallInst(func, args), name);
    }

    auto *createRet(Value *val) {
        return insert(new RetInst(val));
    }

    auto *createRet() {
        return insert(new RetInst());
    }

    auto *createNeg(Value *val, StrView name = "neg") {
        return insert(new NegInst(val), name);
    }

    auto *createNot(Value *val, StrView name = "not") {
        return insert(new NotInst(val), name);
    }

    // simple form of create instruction
    auto *call(Function *func, const std::vector<Value *> &args, StrView name = "call") {
        return insert(new CallInst(func, args), name);
    }

    auto *store(Value *ptr, Value *val) {
        return insert(new StoreInst(ptr, val));
    }

    auto *load(Value *ptr, StrView name = "load") {
        return insert(new LoadInst(ptr), name);
    }

    auto *ret(Value *val) {
        return insert(new RetInst(val));
    }

    auto *bin(BinaryOp op, Value *lhs, Value *rhs, StrView name) {
        return createBinary(op, lhs, rhs, name);
    }


private:
    /// Insert point
    BasicBlock *bb = nullptr;
};


#endif //DRAGONIR_IRBUILDER_H
