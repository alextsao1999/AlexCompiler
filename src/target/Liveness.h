//
// Created by Alex on 2022/5/4.
//

#ifndef DRAGON_LIVENESS_H
#define DRAGON_LIVENESS_H

#include "MachinePass.h"
class Liveness : public MachinePass {
public:
    using BlockSet = std::unordered_map<MachineBlock *, std::set<PatternNode *>>;
    BlockSet liveKills;
    BlockSet liveGens;

    void runOnFunction(Function &function) override {
        computeLocalLiveness(&function);
        computeGlobalLiveness(&function);

        std::cout << "Liveness: " << std::endl;
        std::cout << function.dag.dump() << std::endl;
        std::cout << std::endl;
    }

    void computeLocalLiveness(Function *func) {
        for (auto &MBB: func->blocks) {
            auto &LiveGen = liveGens[&MBB];
            auto &LiveKill = liveKills[&MBB];
            for (auto &Inst: MBB.instrs()) {
                for (auto &Use: Inst.op()) {
                    if (Use.isVirReg()) {
                        if (!LiveKill.count(Use.getOrigin())) {
                            LiveGen.insert(Use.getOrigin());
                        }
                    }
                }
                for (auto &Def : Inst.defs()){
                    if (Def.isVirReg()) {
                        LiveKill.insert(Def.getOrigin());
                    }
                }
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

    void dump(Function *function) {
        for (auto &MBB: function->blocks) {
            std::cout << MBB.name << ":" << std::endl;
            std::cout << "Preds:" << dump_str(MBB.preds, [](MachineBlock *mbb) { return mbb->name; }) << std::endl;
            std::cout << "Succs:" << dump_str(MBB.succs, [](MachineBlock *mbb) { return mbb->name; }) << std::endl;
            std::cout << std::endl;
        }
    }
};

#endif //DRAGON_LIVENESS_H
