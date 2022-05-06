//
// Created by Alex on 2022/3/8.
//

#include "BasicBlock.h"
#include "Function.h"

BasicBlock *BasicBlock::Create(Function *parent, StrView name) {
    assert(parent);
    return new BasicBlock(parent, name);
}

Context *BasicBlock::getContext() const {
    assert(getParent());
    return getParent()->getContext();
}

SymbolTable *BasicBlock::getSymbolTable() const {
    if (auto *F = getParent()) {
        return &F->getSymbolTable();
    }
    return nullptr;
}

BasicBlock::BasicBlock(Function *parent, StrView name)
    : NodeWithParent(parent), name(name) {
    if (auto *ST = getSymbolTable()) {
        // FIXME: Just for allocating count ahead of time
        count = ST->addCount(this, this->name);
    }
}

BasicBlock *BasicBlock::split(Instruction *i, std::string newName) {
    if (newName.empty()) {
        newName = getName() + ".split";
    }
    BasicBlock *NewBB = new BasicBlock(newName);
    insertBeforeThis(NewBB);
    iterator First = begin();
    iterator Last = iterator(i);
    while (First != Last) {
        auto *Inst = &(*First++);
        NewBB->append(Inst);
    }
    replaceAllUsesWith(NewBB);
    NewBB->append(new BranchInst(this));
    return NewBB;

    /*if (newName.empty()) {
        newName = getName() + ".split";
    }
    BasicBlock *NewBB = new BasicBlock(newName);
    insertAfterThis(NewBB);
    iterator First = iterator(i);
    iterator Last = end();
    while (First != Last) {
        auto *Inst = &(*First++);
        NewBB->append(Inst);
    }
    assert(getTerminator() == nullptr);
    append(new BranchInst(NewBB));
    return NewBB;*/
}

bool BasicBlock::isUnreachable() const {
    if (auto *Parent = getParent()) {
        if (this == Parent->getEntryBlock()) {
            return false;
        }
        // No predecessors
        if (preds_begin() == preds_end()) {
            return true;
        }

        // No dominators
        if (dominator == nullptr /*&& domChildren.empty()*/) {
            return true;
        }

        return false;
    }
    assert(!"No parent");
    return false;
}
