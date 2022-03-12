//
// Created by Alex on 2022/3/8.
//

#include "BasicBlock.h"
#include "Function.h"

BasicBlock *BasicBlock::Create(Function *parent, std::string_view name) {
    assert(parent);
    auto *BB = new BasicBlock(parent, name);
    parent->getBasicBlockList().push_back(BB);
    return BB;
}

Context *BasicBlock::getContext() {
    assert(getParent());
    return getParent()->getContext();
}
