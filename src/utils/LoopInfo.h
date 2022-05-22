//
// Created by Alex on 2021/10/26.
//

#ifndef DRAGONCOMPILER_LOOPINFO_H
#define DRAGONCOMPILER_LOOPINFO_H
#include <algorithm>
#include "Instruction.h"
class BasicBlock;

class Loop {
    friend class LoopAnalyse;
    ///< 父层循环
    Loop *parent = nullptr;
    ///< 循环所指向的头
    BasicBlock *header = nullptr;
    ///< 后向边, 可能有多个后向边指向同一个循环头暂时不用
    BasicBlock *backege = nullptr;
    ///< 循环层级
    unsigned level = 0;
    ///< 循环预头, block中不包含循环预头
    BasicBlock *preheader = nullptr;
    std::set<BasicBlock *> blocks;
    std::set<BasicBlock *> exits;
public:
    Loop(BasicBlock *header, BasicBlock *backege) : header(header), backege(backege) {
        assert(header && backege);
        blocks.insert(header);
    }
    // iterator
    inline auto begin() { return blocks.begin(); }
    inline auto end() { return blocks.end(); }

    ///< get the parent
    inline Loop *getParent() { return parent; }
    ///< get the header of the loop
    inline BasicBlock *getHeader() const { return header; }
    ///< get the back edge of the loop
    inline BasicBlock *getBackedge() const { return backege; }
    ///< get the preheader of the loop
    inline BasicBlock *getPreheader() const { return preheader; }
    ///< set the preheader of the loop
    void setPreheader(BasicBlock *ph) {
        this->preheader = ph;
    }
    inline bool contains(BasicBlock *bb) { return blocks.count(bb) > 0; }
    inline bool isLoopInvariant(Value *val) {
        if (auto *Inst = val->as<Instruction>()) {
            if (!contains(Inst->getParent())) {
                return true;
            }
            if (hasLoopInvariantOperands(Inst)) {
                switch (Inst->getOpcode()) {
                    case OpcodeBinary:
                    case OpcodeLoad:
                        return true;
                    default:
                        break;
                }
            }
            return false;
        }
        return true;
    }
    inline bool hasLoopInvariantOperands(Instruction *inst) {
        return std::all_of(inst->begin(), inst->end(), [this](auto &op) {
            return isLoopInvariant(op.getValue());
        });
    }

    inline bool addBlock(BasicBlock *bb) { return blocks.insert(bb).second; }
    inline void removeBlock(BasicBlock *bb) { blocks.erase(bb); }
};

#endif //DRAGONCOMPILER_LOOPINFO_H
