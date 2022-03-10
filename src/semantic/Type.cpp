//
// Created by Alex on 2021/9/4.
//

#include "Type.h"

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
