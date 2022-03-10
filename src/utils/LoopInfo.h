//
// Created by Alex on 2021/10/26.
//

#ifndef DRAGONCOMPILER_LOOPINFO_H
#define DRAGONCOMPILER_LOOPINFO_H

class BasicBlock;

struct LoopInfo {
    LoopInfo *parent = nullptr;
    BasicBlock *header = nullptr; // 循环所指向的头
    BasicBlock *backege = nullptr; // 后向边, 可能有多个后向边指向同一个循环头暂时不用
    unsigned level = 0; // 循环层级
    LoopInfo(BasicBlock *header) : header(header) {}

    inline BasicBlock *getHeader() const {
        return header;
    }

    inline BasicBlock *getBackedge() const {
        return backege;
    }

    inline LoopInfo *getParent() const {
        return parent;
    }

    LoopInfo *getOutermost() {
        auto *Info = this;
        while (auto *Parent = Info->parent)
            Info = Parent;
        return Info;
    }

    unsigned getLevel() {
        return level;
    }

    void calculateLevel() {
        if (parent) {
            if (parent->level == 0) {
                parent->calculateLevel();
            }
            level = parent->level + 1;
        } else {
            level = 1;
        }
    }
};


#endif //DRAGONCOMPILER_LOOPINFO_H
