//
// Created by Alex on 2022/5/7.
//

#ifndef DRAGON_TARGET_H
#define DRAGON_TARGET_H

#include <MachineInstr.h>
#include <PatternDAG.h>
#include <Constant.h>
#include <functional>
class TargetInfo {
public:
    TargetInfo() = default;
    virtual ~TargetInfo() = default;

    virtual bool isMove(const MachineInstr &instr) const {
        return instr.getOpcode() == TargetOpcode::TargetMove;
    }

    virtual bool isBranch(const MachineInstr &instr) const {
        return instr.getOpcode() == TargetOpcode::TargetBr || instr.getOpcode() == TargetOpcode::TargetCBr;
    }

    virtual bool isCall(const MachineInstr &instr) const {
        return instr.getOpcode() == TargetOpcode::TargetCall;
    }

    virtual bool isEliminableMove(const MachineInstr &instr) const {
        if (instr.getOpcode() == TargetBr) {
            if (instr.use_begin()->getLabel() == instr.getParent()->getNext()) {
                return true;
            }
        }
        if (instr.getOpcode() == TargetOpcode::TargetMove && instr.hasDef()) {
            for (auto &Def: instr.defs()) {
                for (auto &Use: instr.uses()) {
                    if (Def != Use) {
                        return false;
                    }
                }
            }
            return true;
        }
        return false;
    }

    virtual const char *getRegName(unsigned regNo) const {
        const char *regNames[] = {
                "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
        };
        return regNames[regNo];
    }

    virtual const std::vector<Register> &getTempRegList() const {
        static std::vector<Register> tempRegList = {0, 1, 2, 3, 4, 5, 6, 7};
        return tempRegList;
    }

    virtual const std::vector<Register> &getSaveRegList() const {
        static std::vector<Register> saveRegList = {8, 9, 10, 11, 12, 13, 14, 15};
        return saveRegList;
    }

    virtual bool isSaveReg(Register reg) const {
        return std::find(getSaveRegList().begin(), getSaveRegList().end(), reg) != getSaveRegList().end();
    }

    virtual PatternNode *loweringArgument(PatternDAG &DAG, Param *param, int i) const {
        return DAG.getReg(Register::phy(i));
    }

    virtual PatternNode *loweringReturn(PatternDAG &DAG, PatternNode *val) const {
        if (!val) {
            return nullptr;
        }
        return DAG.getCopyToReg(DAG.getReg(Register::phy(0)), val);
    }

};


#endif //DRAGON_TARGET_H
