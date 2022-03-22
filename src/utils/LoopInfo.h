//
// Created by Alex on 2021/10/26.
//

#ifndef DRAGONCOMPILER_LOOPINFO_H
#define DRAGONCOMPILER_LOOPINFO_H
#include <algorithm>
#include "Instruction.h"
class BasicBlock;

class Loop {
    std::set<BasicBlock *> blocks;
public:
    inline auto begin() { return blocks.begin(); }
    inline auto end() { return blocks.end(); }

    inline bool contains(BasicBlock *bb) { return blocks.count(bb) > 0; }
    inline bool isLoopInvariant(Value *val) {
        if (auto *Inst = val->as<Instruction>()) {
            return !contains(Inst->getParent());
        }
        return true;
    }
    inline bool hasLoopInvariantOperands(Instruction *inst) {
        return std::all_of(inst->begin(), inst->end(), [this](auto &op) {
            return isLoopInvariant(op.getValue());
        });
    }

    inline void addBlock(BasicBlock *bb) { blocks.insert(bb); }
    inline void removeBlock(BasicBlock *bb) { blocks.erase(bb); }
};

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
