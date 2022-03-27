//
// Created by Alex on 2022/3/24.
//

#ifndef DRAGON_IDFCALCULATOR_H
#define DRAGON_IDFCALCULATOR_H
#include <functional>
#include <queue>
#include <vector>
#include "PassManager.h"
#include "Function.h"
template<typename T>
inline auto pop_back_val(T &c) {
    auto V = c.back();
    c.pop_back();
    return V;
}

inline void SubTreeIterate(BasicBlock *bb, std::function<void(BasicBlock *)> iter) {
    std::vector<BasicBlock *> Worklist;
    Worklist.push_back(bb);
    while (!Worklist.empty()) {
        BasicBlock *Cur = pop_back_val(Worklist);
        iter(Cur);
        for (auto *Succ: Cur->getDomChildren()) {
            Worklist.push_back(Succ);
        }
    }
}

struct BasicBlockCompare {
    bool operator()(const BasicBlock *l, const BasicBlock *r) const {
        return l->getLevel() >= r->getLevel();
    }
};

/**
 * The iterated dominance frontier calculator.
 */
class IDFCalculator {
public:
    std::set<BasicBlock *> visited;
    std::map<unsigned, std::priority_queue<BasicBlock *, std::vector<BasicBlock *>, BasicBlockCompare>> DAT;
    std::set<BasicBlock *> S;
    BasicBlock *currentNode = nullptr;
    std::set<BasicBlock *> IDF;

    void calulate(const std::vector<BasicBlock *> &blocks) {
        DAT.clear();
        S.clear();
        visited.clear();
        IDF.clear();

        for (auto &BB: blocks) {
            DAT[BB->getLevel()].push(BB);
            S.insert(BB);
        }

        while (currentNode = getDeepestNode()) {
            visited.insert(currentNode);
            visit(currentNode);
        }
    }

    void insertNode(BasicBlock *bb) {
        DAT[bb->getLevel()].push(bb);
    }

    BasicBlock *getDeepestNode() {
        for (auto Iter = DAT.rbegin(); Iter != DAT.rend(); ++Iter) {
            if (!Iter->second.empty()) {
                auto *BB = Iter->second.top();
                Iter->second.pop();
                return BB;
            }
        }
        return nullptr;
    }

    void visit(BasicBlock *bb) {
        for (auto *Succ: bb->succs()) {
            if (bb->getDomChildren().count(Succ)) {
                continue;
            }
            if (Succ->hasMultiplePredecessor() && Succ->getLevel() <= currentNode->getLevel()) {
                if (!IDF.count(Succ)) {
                    IDF.insert(Succ);
                    if (!S.count(Succ)) {
                        insertNode(Succ);
                    }
                }
            }
        }
        for (auto &DEdge : bb->getDomChildren()) {
            if (!visited.count(DEdge)) {
                visited.insert(DEdge);
                visit(DEdge);
            }
        }
    }

    std::set<BasicBlock *> calc(const std::set<BasicBlock *> &blocks) {
        std::priority_queue<BasicBlock *, std::vector<BasicBlock *>, BasicBlockCompare> Queue;
        std::vector<BasicBlock *> Worklist;
        std::set<BasicBlock *> Visited;
        std::set<BasicBlock *> IDFSet;
        for (auto &BB: blocks) {
            Queue.push(BB);
            Visited.insert(BB);
        }

        while (!Queue.empty()) {
            auto *Working = Queue.top();
            Queue.pop();

            Visited.insert(Working);
            Worklist.push_back(Working);

            while (!Worklist.empty()) {
                auto *BB = Worklist.back();
                Worklist.pop_back();

                for (auto *Succ: BB->succs()) {
                    if (Working->getDomChildren().count(Succ)) {
                        // make sure working -> succ is not d edge
                        continue;
                    }

                    if (Succ->getLevel() > Working->getLevel()) {
                        continue;
                    }

                    if (!IDFSet.insert(Succ).second) {
                        continue;
                    }

                    if (!blocks.count(Succ)) {
                        Queue.push(Succ);
                    }
                }

                // 沿着d边继续行进
                for (auto *DEdge: BB->getDomChildren()) {
                    if (Visited.insert(DEdge).second) {
                        Worklist.push_back(DEdge);
                    }
                }

            }

        }
        return IDFSet;
    }

};


#endif //DRAGON_IDFCALCULATOR_H
