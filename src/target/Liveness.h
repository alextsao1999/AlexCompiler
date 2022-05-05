//
// Created by Alex on 2022/5/4.
//

#ifndef DRAGON_LIVENESS_H
#define DRAGON_LIVENESS_H

#include "MachinePass.h"
class Liveness : public MachineBlockPass {
public:
    using BlockSet = std::unordered_map<MachineBlock *, std::set<PatternNode *>>;
    BlockSet liveKills;
    BlockSet liveGens;

    void runOnFunction(Function &function) override {
        computeLocalLiveness(&function);
        computeGlobalLiveness(&function);
    }

    void computeLocalLiveness(Function *func) {
        for (auto &MBB: func->blocks) {
            auto &liveGen = liveGens[&MBB];
            auto &liveKill = liveKills[&MBB];
            for (auto &inst: MBB.instrs()) {
                /*for (auto &use: inst.getUses()) {
                    if (use->isVirReg() || use->isArgument()) {
                        if (!liveKill.count(use)) {
                            liveGen.insert(use);
                        }
                    }
                }
                if (auto *Def = inst.getDef()) {
                    if (Def->isVirReg() || Def->isArgument()) {
                        liveKill.insert(Def);
                    }
                }*/
            }
        }
    }

    void computeGlobalLiveness(Function *func) {
        bool Changed;
        do {
            Changed = false;
            for (auto &MBB: func->blocks) {
                auto &LiveOut = MBB.liveOutSet;
                for (auto *Succ: MBB.succs) {
                    auto &SuccLiveKill = liveKills[Succ];
                    for (auto &Use: liveGens[Succ]) {
                        if (LiveOut.insert(Use).second) {
                            Changed = true;
                        }
                    }
                    for (auto &Def: Succ->liveOutSet) {
                        if (!SuccLiveKill.count(Def)) {
                            if (LiveOut.insert(Def).second) {
                                Changed = true;
                            }
                        }
                    }
                }
            }
        } while (Changed);
    }

    /*void dump(Function *function) {
        for (auto &MBB: function->MBBs) {
            std::cout << MBB->name << ":" << std::endl;
            std::cout << "Preds:" << MBB->preds << std::endl;
            std::cout << "Succs:" << MBB->succs << std::endl;
            dump_os(MBB->liveOutSet, [](PatternTree *node) -> auto & {
                return node->pretty(std::cout);
            });
            std::cout << std::endl;
        }
    }*/
};


#endif //DRAGON_LIVENESS_H
