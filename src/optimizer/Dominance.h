//
// Created by Alex on 2022/3/11.
//

#ifndef DRAGON_DOMINANCE_H
#define DRAGON_DOMINANCE_H

#include <functional>
#include <numeric>
#include "PassManager.h"
#include "Function.h"
#include "BasicBlock.h"
class Dominance : public FunctionPass {
public:
    std::set<BasicBlock *> visited;
    std::vector<BasicBlock *> bbReorder;
    std::map<BasicBlock *, size_t> bbIndex;

    void runOnFunction(Function *function) override {
        visited.clear();
        bbIndex.clear();
        bbReorder.clear();

        auto *EntryBlock = function->getEntryBlock();
        assert(EntryBlock);
        // compute reverse post order
        RPO(EntryBlock);

        // compute immediate dominator
        bool Changed;
        do {
            Changed = false;
            for (auto &BB: iter_reverse(bbReorder)) {
                BasicBlock *IDom = nullptr;
                for (auto *Pred: BB->preds()) {
                    if (IDom == nullptr) {
                        IDom = Pred;
                    } else {
                        IDom = intersect(IDom, Pred);
                    }
                }

                if (IDom != BB->getDominator()) {
                    BB->setDominator(IDom);
                    Changed = true;
                }

            }
        } while (Changed);

        // dominance frontier
        for (auto &BB: bbReorder) {
            if (BB->hasMultiplePredecessor()) {
                for (auto *Pred: BB->preds()) {
                    auto *Runner = Pred;
                    while (Runner && Runner != BB->getDominator()) {
                        Runner->addDomFrontier(BB);
                        Runner = Runner->getDominator();
                    }
                }
            }
        }

        for (auto &Item : bbReorder) {
            if (auto *Dom = Item->getDominator()) {
                Dom->domChildren.insert(Item);
            }
        }

        // calculate level
        EntryBlock->calculateLevel();
    }

    void RPO(BasicBlock *bb) {
        visited.insert(bb);
        for (auto *Succ: bb->succs()) {
            if (visited.find(Succ) == visited.end()) {
                RPO(Succ);
            }
        }
        bbIndex[bb] = bbReorder.size();
        bbReorder.push_back(bb);
        bb->clearDomInfo();
    }

    BasicBlock *intersect(BasicBlock *b1, BasicBlock *b2) {
        while (b1 != b2) {
            while (bbIndex[b1] < bbIndex[b2]) {
                b1 = b1->dominator;
                //assert(b1);
                if (!b1)
                    return b2;
            }
            while (bbIndex[b2] < bbIndex[b1]) {
                b2 = b2->dominator;
                //assert(b2);
                if (!b2)
                    return b1;
            }
        }
        return b1;
    }

};


#endif //DRAGON_DOMINANCE_H
