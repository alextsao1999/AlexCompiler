//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_LOOPSIMPLIFY_H
#define DRAGON_LOOPSIMPLIFY_H

#include "PassManager.h"
#include "LoopAnalyse.h"

class LoopSimplify : public LoopPass {
public:
    void runOnLoop(Loop *loop) override {
        if (auto *Header = loop->getHeader()) {
            if (!Header->hasMultiplePredecessor()) {
                return;
            }
            auto *F = Header->getParent();
            assert(F);
            auto *NewPreheader = new BasicBlock(Header->getName() + ".preheader");
            F->insertBefore(Header, NewPreheader);

            for (auto I = Header->use_begin(); I != Header->use_end(); ) {
                auto &Use = *I++;
                if (auto *Instr = Use.getUser()->as<Instruction>()) {
                    if (!loop->contains(Instr->getParent())) {
                        Use.set(NewPreheader);
                    }
                }
            }

            // append branch to loop header
            NewPreheader->append(new BranchInst(Header));
        }
    }
};


#endif //DRAGON_LOOPSIMPLIFY_H
