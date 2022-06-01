//
// Created by Alex on 2021/9/4.
//

#include "Type.h"
#include "Context.h"

Type *Type::getPointerType() {
    ASSERT(getContext());
    return getContext()->getPointerTy(this);
}
