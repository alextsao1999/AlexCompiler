//
// Created by Alex on 2022/5/8.
//

#ifndef DRAGON_RISCVTARGET_H
#define DRAGON_RISCVTARGET_H

#include <Target.h>
#include <Common.h>
#define RV32I(S)  \
S(SLL)            \
S(SLLI)           \
S(SRL)            \
S(SRLI)           \
S(SRA)            \
S(SRAI)           \
S(ADD)            \
S(ADDI)           \
S(SUB)            \
S(LUI)            \
S(AUIPC)          \
S(XOR)            \
S(XORI)           \
S(OR)             \
S(ORI)            \
S(AND)            \
S(ANDI)           \
S(SLT)            \
S(SLTI)           \
S(SLTU)           \
S(SLTIU)          \
S(BEQ)            \
S(BNE)            \
S(BLT)            \
S(BGE)            \
S(BLTU)           \
S(BGEU)           \
S(JAL)            \
S(JALR)           \
S(FENCE)          \
S(FENCE_I)        \
S(ECALL)          \
S(EBREAK)         \
S(CSRRW)          \
S(CSRRS)          \
S(CSRRC)          \
S(CSRRWI)         \
S(CSRRSI)         \
S(CSRRCI)         \
S(LB)             \
S(LH)             \
S(LBU)            \
S(LHU)            \
S(LW)             \
S(SB)             \
S(SH)             \
S(SW)

#define RV32M(S) \
S(MUL)           \
S(MULH)          \
S(MULHSU)        \
S(DIV)           \
S(DIVU)          \
S(REM)           \
S(REMU)

#define RV64I(S) \
S(SLLW)          \
S(SLLIW)         \
S(SRLW)          \
S(SRLIW)

#define RV64M(S) \
S(MULW)          \
S(DIVW)          \
S(REMW)          \
S(REMUW)

#define RVPseudo(S) \
S(ARG)              \
S(MV)               \
S(LI)               \
S(J)                \
S(RET)              \
S(BLE)              \
S(BGT)              \
S(CALL)

#define RegABINames(S) \
S(Zero, X0)            \
S(Ra, X1)              \
S(Sp, X2)              \
S(Gp, X3)              \
S(Tp, X4)              \
S(T0, X5)              \
S(T1, X6)              \
S(T2, X7)              \
S(Fp, X8)              \
S(S1, X9)              \
/*  */                 \
S(A0, X10)             \
S(A1, X11)             \
S(A2, X12)             \
S(A3, X13)             \
S(A4, X14)             \
S(A5, X15)             \
S(A6, X16)             \
S(A7, X17)             \
S(S2, X18)             \
S(S3, X19)             \
S(S4, X20)             \
S(S5, X21)             \
S(S6, X22)             \
S(S7, X23)             \
S(S8, X24)             \
S(S9, X25)             \
S(S10, X26)            \
S(S11, X27)            \
/* Temp */             \
S(T3, X28)             \
S(T4, X29)             \
S(T5, X30)             \
S(T6, X31)

namespace RISCV {
    enum RegList {
        X0, X1, X2, X3, X4, X5,
        X6, X7, X8, X9, X10, X11,
        X12, X13, X14, X15, X16,
        X17, X18, X19, X20, X21,
        X22, X23, X24, X25, X26,
        X27, X28, X29, X30, X31,
        // ABI Name
#define ABIDef(Alias, Reg) Alias = Reg,
        RegABINames(ABIDef)
        S0 = Fp,
    };

    enum Opcode {
        RiscvFirstOpcode = TargetOpcode::TargetLastOpcode,
#define RISCVOpcode(OPCODE) OPCODE,
        RV32I(RISCVOpcode)
        RV32M(RISCVOpcode)
        RV64I(RISCVOpcode)
        RV64M(RISCVOpcode)

        // Pseudo Opcode
        RVPseudo(RISCVOpcode)
    };

    inline std::ostream &dump_opcode(std::ostream &os, unsigned Opcode) {
#define DumpRISCV(OPCODE) if (OPCODE == Opcode) os << strlower(#OPCODE);
        RV32I(DumpRISCV)
        RV32M(DumpRISCV)
        RV64I(DumpRISCV)
        RV64M(DumpRISCV)
        RVPseudo(DumpRISCV)
        return os;
    }

} // namespace RISCV

class RISCVTarget : public TargetInfo {
public:
    const char *getRegName(unsigned int regNo) const override {
        switch (regNo) {
#define RegABIName(Alias, Reg) case RISCV::Reg: return #Alias;
            RegABINames(RegABIName)
            default:
                break;
        }
        return "Unknown";
    }

    const std::vector<Register> &getTempRegList() const override {
        static const std::vector<Register> tempRegList = {
                RISCV::T0, RISCV::T1, RISCV::T2, RISCV::T3, RISCV::T4, RISCV::T5, RISCV::T6
        };
        return tempRegList;
    }

    const std::vector<Register> &getSaveRegList() const override {
        static const std::vector<Register> saveRegList = {
                RISCV::S1, RISCV::S2, RISCV::S3, RISCV::S4, RISCV::S5, RISCV::S6,
                RISCV::S7, RISCV::S8, RISCV::S9, RISCV::S10, RISCV::S11,
        };
        return saveRegList;
    }

    bool isMove(const MachineInstr &instr) const override {
        auto Opcode = instr.getOpcode();
        if (Opcode == RISCV::MV || Opcode == TargetMove) {
            return true;
        }
        if (Opcode == RISCV::ADDI) {
            return instr.getOp(1).getReg() == RISCV::Zero;
        }
        return false;
    }

    PatternNode *loweringArgument(PatternDAG &DAG, Param *param, int i) const override {
        return TargetInfo::loweringArgument(DAG, param, i);
    }

    PatternNode *loweringReturn(PatternDAG &DAG, PatternNode *val) const override {
        return TargetInfo::loweringReturn(DAG, val);
    }

};


#endif //DRAGON_RISCVTARGET_H
