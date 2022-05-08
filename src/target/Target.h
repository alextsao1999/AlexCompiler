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
        return false;
    }

    virtual bool isBranch(const MachineInstr &instr) const {
        return false;
    }

    virtual bool isCall(const MachineInstr &instr) const {
        return false;
    }

    std::vector<Register> getTempRegList() const {
        return {};
    }

    std::vector<Register> getSaveRegList() const {
        return {};
    }

    virtual PatternNode *loweringArgument(PatternDAG &DAG, Param *param, int i) const {
        return DAG.getPhyReg(i);
    }

    virtual PatternNode *loweringReturn(PatternDAG &DAG, PatternNode *val) const {
        if (!val) {
            return nullptr;
        }
        return DAG.getCopyToReg(DAG.getPhyReg(0), val);
    }

};


#endif //DRAGON_TARGET_H
