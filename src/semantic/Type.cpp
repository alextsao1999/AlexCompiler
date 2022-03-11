//
// Created by Alex on 2021/9/4.
//

#include "Type.h"
#include "Context.h"

unsigned Type::getBitSize() {
    switch (getTypeId()) {
        case TypeVoid:
            return 0;
        case TypeInt:
            return static_cast<IntegerType *>(this)->getBitSize();
        default:
            break;
    }
    unreachable();
    return 0;
}

Type *Type::getPointerType() {
    assert(getContext());
    return getContext()->getPointerTy(this);
}
