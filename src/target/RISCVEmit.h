//
// Created by Alex on 2022/5/9.
//

#ifndef DRAGON_RISCVEMIT_H
#define DRAGON_RISCVEMIT_H

#include "MachinePass.h"
#include "RISCVTarget.h"

class RISCVEmit : public MachinePass {
public:
    std::stringstream ss;
    TargetInfo *TI = nullptr;
    Function *F = nullptr;
    int saveRegCount = 0;
    void runOnFunction(Function &function) override {
        F = &function;
        TI = function.getTargetInfo();
        for (auto &block: function.blocks) {
            ss << getBlockLable(block) << ":\n";
            if (block.getOrigin() == function.getEntryBlock()) {
                emitPushSaveReg();
            }
            for (auto &inst: block) {
                emit(inst);
            }
        }
    }

    void emit(MachineInstr &instr) {
        if (instr.getOpcode() == TargetOpcode::TargetRet) {
            emitPopSaveReg();
        }
        indent(4);
        dumpOpcode(instr.getOpcode());
        ss << " ";
        ss << dump_str(instr.ops(), [&](Operand &op) {
            if (op.isReg()) {
                return strlower(TI->getRegName(op.getReg()));
            }
            if (op.isImm()) {
                return std::to_string(op.getImm());
            }
            if (op.isLabel()) {
                return getBlockLable(*op.getLabel());
            }
            if (op.isSlot()) {
                return "slot" + std::to_string(op.value.slot);
            }
            return std::string();
        });
        ss << std::endl;
    }

    void dumpOpcode(unsigned opcode) {
        switch (opcode) {
            case TargetAdd:
                ss << "add";
                break;
            case TargetSub:
                ss << "sub";
                break;
            case TargetMul:
                ss << "mul";
                break;
            case TargetDiv:
                ss << "div";
                break;
            case TargetMod:
                ss << "mod";
                break;
            case TargetAnd:
                ss << "and";
                break;
            case TargetOr:
                ss << "or";
                break;
            case TargetXor:
                ss << "xor";
                break;
            case TargetShl:
                ss << "shl";
                break;
            case TargetShr:
                ss << "shr";
                break;
            case TargetEq:
                ss << "eq";
                break;
            case TargetNe:
                ss << "ne";
                break;
            case TargetLt:
                ss << "lt";
                break;
            case TargetGt:
                ss << "gt";
                break;
            case TargetLe:
                ss << "le";
                break;
            case TargetGe:
                ss << "ge";
                break;
            case TargetNeg:
                ss << "neg";
                break;
            case TargetNot:
                ss << "not";
                break;
            case TargetMove:
                ss << "move";
                break;
            case TargetCBr:
                ss << "cbr";
                break;
            case TargetBr:
                ss << "br";
                break;
            case TargetCall:
                ss << "call";
                break;
            case TargetLoad:
                ss << "load";
                break;
            case TargetStore:
                ss << "store";
                break;
            case TargetCmp:
                ss << "cmp";
                break;
            case TargetRet:
                ss << "ret";
                break;
            case TargetNone:
                ss << "none";
                break;
            default:
                RISCV::dump_opcode(ss, opcode);
                break;
        }
    }

    void emitPushSaveReg() {
        saveRegCount = 0;
        for (auto &Reg: F->allocatedRegs) {
            if (TI->isSaveReg(Reg)) {
                indent() << "sw " << strlower(TI->getRegName(Reg)) << ", " << ((saveRegCount++) * 4) << "(sp)\n";
            }
        }
    }
    void emitPopSaveReg() {
        saveRegCount = 0;
        for (auto &Reg: F->allocatedRegs) {
            if (TI->isSaveReg(Reg)) {
                indent() << "lw " << strlower(TI->getRegName(Reg)) << ", " << ((saveRegCount++) * 4) << "(sp)\n";
            }
        }
    }

    std::string getBlockLable(MachineBlock &block) {
        std::string name = block.name;
        std::replace(name.begin(), name.end(), '.', '_');
        std::replace(name.begin(), name.end(), '%', '_');
        return F->getName() + name;
    }

    std::ostream &indent(int size = 4) {
        return ss << std::string(size, ' ');
    }

    std::string str() {
        return ss.str();
    }

};


#endif //DRAGON_RISCVEMIT_H
