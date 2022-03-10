//
// Created by Alex on 2022/3/8.
//

#include "Value.h"
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

