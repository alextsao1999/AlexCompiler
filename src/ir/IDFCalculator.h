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
        return l->getLevel() < r->getLevel();
    }
};

/**
 * The iterated dominance frontier calculator.
 */
class IDFCalculator {
public:
    std::set<BasicBlock *> visited;
    std::map<unsigned, std::priority_queue<BasicBlock *, std::vector<BasicBlock *>, BasicBlockCompare>> DAT;

    void calulate(const std::set<BasicBlock *> &blocks) {
        for (auto &BB: blocks) {
            DAT[BB->getLevel()].push(BB);
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
                if (IDF.count(Succ)) {
                    insertNode(Succ);
                } else {
                    IDF.insert(Succ);
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

    bool isJEdge(BasicBlock *from, BasicBlock *to) {
        if (from->getDomChildren().count(to)) { // is d edge
            return false;
        }

        for (auto *Succ: from->succs()) {
            if (Succ == to && Succ->hasMultiplePredecessor()) {
                return true;
            }
        }
        return false;
    }
    BasicBlock *currentNode = nullptr;
    std::set<BasicBlock *> IDF;
    BasicBlock *entry;
};


class IDF: public FunctionPass {
public:
    void runOnFunction(Function *function) override {
        function->getEntryBlock()->calculateLevel();
        IDFCalculator Calc;

        auto *Bbtrue = function->getBlockByName("if.true");
        auto *Bbfalse = function->getBlockByName("if.false");

        if (Bbtrue && Bbfalse) {
            Calc.calulate({Bbtrue, Bbfalse});
            int i = 0;

        }

    }

};

#endif //DRAGON_IDFCALCULATOR_H
