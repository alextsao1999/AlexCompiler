//
// Created by Alex on 2022/3/8.
//

#include "Value.h"
#include "Instruction.h"
#include "Function.h"

Value::Value() {
    //std::cout << "alloc" << this << std::endl;
}

Value::~Value() {
    // replaceAllUsesWith(nullptr);
}

bool Value::isOnlyUsedOnce() const {
    return users && users->next == nullptr;
}

bool Value::isNotUsed() const {
    return users == nullptr;
}

void Value::replaceAllUsesWith(Value *newVal) {
    Use *Cur = users;
    while (Cur) {
        auto *Next = Cur->getNext();
        Cur->set(newVal);
        Cur = Next;
    }
}

Opcode Value::getOpcode() {
    assert(isa<Instruction>());
    return cast<Instruction>()->getOpcode();
}

bool Value::isFunction() const {
    return isa<Function>();
}

bool Value::isConstant() const {
    return isa<Constant>();
}

bool Value::isConstantZero() const {
    if (auto *Val = as<IntConstant>()) {
        return Val->getVal() == 0;
    }
    return false;
}

bool Value::isBasicBlock() const {
    return isa<BasicBlock>();
}

bool Value::isInstruction() const {
    return isa<Instruction>();
}
