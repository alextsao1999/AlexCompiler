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

bool Value::isOnlyUsedOnce() {
    return users && users->next == nullptr;
}

Opcode Value::getOpcode() {
    assert(isa<Instruction>());
    return cast<Instruction>()->getOpcode();
}

void Value::replaceAllUsesWith(Value *newVal) {
    Use *Cur = users;
    while (Cur) {
        auto *Next = Cur->getNext();
        Cur->set(newVal);
        Cur = Next;
    }
}

