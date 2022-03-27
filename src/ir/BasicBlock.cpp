//
// Created by Alex on 2022/3/8.
//

#include "BasicBlock.h"
#include "Function.h"

BasicBlock *BasicBlock::Create(Function *parent, std::string_view name) {
    assert(parent);
    return new BasicBlock(parent, name);
}

Context *BasicBlock::getContext() {
    assert(getParent());
    return getParent()->getContext();
}
