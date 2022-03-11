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
        bbReorder.clear();
        // compute reverse post order
        std::function<void(BasicBlock * bb)> DFS = [&](BasicBlock *bb) {
            visited.insert(bb);
            for (auto &Succ: bb->getSuccessors()) {
                if (visited.find(Succ) == visited.end()) {
                    DFS(Succ);
                }
            }
            bbIndex[bb] = bbReorder.size();
            bbReorder.push_back(bb);
        };

        DFS(function->getEntryBlock());

        // compute immediate dominator
        bool Changed;
        do {
            Changed = false;
            for (auto &BB: iter_reverse(bbReorder)) {
                BasicBlock *IDom = nullptr;
                for (auto &Pred: BB->getPredecessors()) {
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

        /*for (auto &BB: bbReorder) {
            for (auto &Succ: BB->getSuccessors()) {
                if (BB->getDominator() != Succ) {
                    Succ->addDF(BB);
                }
            }
        }*/

        for (auto &BB: bbReorder) {
            if (BB->getPredecessors().size() >= 2) {
                for (auto &Pred: BB->getPredecessors()) {
                    auto *Runner = Pred;
                    while (Runner != BB->getDominator()) {
                        Runner->addDF(BB);
                        Runner = Runner->getDominator();
                    }
                }
            }
        }

        for (auto &Item : bbReorder) {
            if (auto *Dom = Item->getDominator()) {
                Dom->doms.insert(Item);
            }
        }

    }

    BasicBlock *intersect(BasicBlock *b1, BasicBlock *b2) {
        while (b1 != b2) {
            while (bbIndex[b1] < bbIndex[b2]) {
                b1 = b1->dominator;
            }
            while (bbIndex[b2] < bbIndex[b1]) {
                b2 = b2->dominator;
            }
        }
        return b1;
    }




};


#endif //DRAGON_DOMINANCE_H
