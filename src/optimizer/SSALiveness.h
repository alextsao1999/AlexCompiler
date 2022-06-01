//
// Created by Alex on 2022/5/8.
//

#ifndef DRAGON_SSALIVENESS_H
#define DRAGON_SSALIVENESS_H
#include "PassManager.h"
#include "Function.h"
///< Compute the liveness of ssa.
class SSALiveness : public FunctionPass {
public:
    std::map<BasicBlock *, std::set<Value *>> liveOut;
    std::map<BasicBlock *, std::set<Value *>> liveIn;
    SSALiveness() {}

    void runOnFunction(Function &function) override {

    }
    void setBasicBlockLiveIn(BasicBlock *block, std::set<Value *> in) {
        this->liveIn[block] = in;
    }
    void setBasicBlockLiveOut(BasicBlock *block, std::set<Value *> out) {
        this->liveOut[block] = out;
    }
    std::set<Value *> &getBasicBlockLiveIn(BasicBlock *block) {
        return this->liveIn[block];
    }
    std::set<Value *> &getBasicBlockLiveOut(BasicBlock *block) {
        return this->liveOut[block];
    }

    void computeLivenessOnBlock(BasicBlock &block) {
        std::set<Value *> Live;
        for (auto *Succ: block.succs()) {
            for (auto &Phi: Succ->phis()) {
                Value *V = Phi.findIncomingValue(&block);
                ASSERT(V);
                Live.insert(V);
            }
            auto &SuccLiveIn = getBasicBlockLiveIn(Succ);
            Live.insert(SuccLiveIn.begin(), SuccLiveIn.end());
        }

        for (auto &I: block.instrs().reverse()) {
            Live.erase(&I);
            for (auto *Op: I.ops()) {
                Live.insert(Op);
            }
        }

        setBasicBlockLiveIn(&block, Live);
    }

};


#endif //DRAGON_SSALIVENESS_H
