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
        assert(!f->getSubList().empty());
    }

    void setInsertPoint(Function *f) {
        assert(!f->getSubList().empty());
        bb = &f->getSubList().back();
    }

    void setInsertPoint(BasicBlock *b) { this->bb = b; }

    template<typename Ty>
    Ty *insert(Ty *instr, std::string_view name) {
        assert(instr);
        bb->append(instr);
        instr->setName(name);
        return instr;
    }

    template<typename Ty>
    Ty *insert(Ty *instr) {
        assert(instr);
        bb->append(instr);
        return instr;
    }

    Context *getContext() {
        return bb->getParent()->getParent()->getContext();
    }

    auto *getUndef() {
        return getContext()->getUndef();
    }

    auto *getInt(int64_t val) {
        return getContext()->getInt(val);
    }

    auto *createBr(BasicBlock *target) {
        return insert(new BranchInst(target), "br");
    }

    auto *createAlloca(Type *ty, std::string_view name) {
        return insert(new AllocaInst(ty), name);
    }

    auto *createAdd(Value *lhs, Value *rhs, std::string_view name = "add") {
        return insert(new BinaryInst(Add, lhs, rhs), name);
    }

    auto *createSub(Value *lhs, Value *rhs, std::string_view name = "sub") {
        return insert(new BinaryInst(Sub, lhs, rhs), name);
    }

    auto *createMul(Value *lhs, Value *rhs, std::string_view name = "mul") {
        return insert(new BinaryInst(Mul, lhs, rhs), name);
    }

    auto *createDiv(Value *lhs, Value *rhs, std::string_view name = "div") {
        return insert(new BinaryInst(Div, lhs, rhs), name);
    }

    auto *createLoad(Value *ptr, std::string_view name = "load") {
        return insert(new LoadInst(ptr), name);
    }

    auto *createStore(Value *ptr, Value *val) {
        return insert(new StoreInst(ptr, val));
    }

    auto *createRet(Value *val) {
        return insert(new RetInst(val));
    }

private:
    BasicBlock *bb;
};


#endif //DRAGONIR_IRBUILDER_H
