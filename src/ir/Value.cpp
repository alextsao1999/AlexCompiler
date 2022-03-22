//
// Created by Alex on 2022/3/8.
//

#include "Value.h"
#include "Instruction.h"

Value::Value() {
    //std::cout << "alloc" << this << std::endl;
}

Value::~Value() {
    Use *Cur = users;
    while (Cur) {
        auto *Next = Cur->getNext();
        Cur->unset();
        Cur = Next;
    }
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
