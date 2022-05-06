//
// Created by Alex on 2022/5/7.
//

#ifndef DRAGON_TARGET_H
#define DRAGON_TARGET_H

#include <MachineInstr.h>
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

};


#endif //DRAGON_TARGET_H
