//
// Created by Alex on 2022/3/24.
//

#ifndef DRAGON_MACHINEINSTR_H
#define DRAGON_MACHINEINSTR_H

#include <list>
#include "PatternNode.h"

class Operand : public Node<Operand> {
public:
    static Operand undefined() {
        return Operand(Nop, Register());
    }
    static Operand reg(Register reg) {
        return Operand(Reg, reg);
    }
    static Operand imm(int64_t value) {
        Operand Op;
        Op.kind = Imm;
        Op.value.imm = value;
        return Op;
    }
public:
    enum Kind {
        Nop,
        Reg,
        Imm,
    };
    Kind kind = Nop;

    union Content {
        int64_t imm;
        float fimm;
        double dimm;
    } value;

    Register regOp;
    Operand() {}
    Operand(Kind kind) : kind(kind) {}
    Operand(Kind kind, Register reg) : kind(kind), regOp(reg) {}

    Kind getKind() const {
        return kind;
    }
    bool isReg() const {
        return kind == Reg;
    }
    bool isVirReg() const {
        return isReg() && regOp.isVirReg();
    }
    bool isPhyReg() const {
        return isReg() && regOp.isPhyReg();
    }

    Register getReg() {
        return regOp;
    }
};

class MachineBlock;

enum TargetOpcode {
    TargetNone,
    TargetAdd = Pattern::Add,
    TargetSub = Pattern::Sub,
    TargetMul = Pattern::Mul,
    TargetDiv = Pattern::Div,
    TargetMod = Pattern::Mod,
    TargetRem = Pattern::Rem,
    TargetAnd = Pattern::And,
    TargetOr = Pattern::Or,
    TargetXor = Pattern::Xor,
    TargetShl = Pattern::Shl,
    TargetShr = Pattern::Shr,
    TargetEq = Pattern::Eq,
    TargetNe = Pattern::Ne,
    TargetLt = Pattern::Lt,
    TargetGt = Pattern::Gt,
    TargetLe = Pattern::Le,
    TargetGe = Pattern::Ge,
    TargetMove = Pattern::LastBinOp,
    TargetCBr,
    TargetBr,
    TargetCall,
    TargetCmp,
    TargetRet,
    TargetLastOpcode,
};
class MachineInstr : public NodeWithParent<MachineInstr, MachineBlock> {
public:
    using iterator = NodeList<Operand>::iterator;
    unsigned opcode = 0;
    Operand def;
    NodeList<Operand> operands;
    MachineInstr() {}
    MachineInstr(unsigned opcode) : opcode(opcode) {}

    unsigned getOpcode() const {
        return opcode;
    }

    bool hasDef() {
        return def.kind != Operand::Nop;
    }
    void setDef(Operand d) {
        def = d;
    }

    auto defs() {
        return iter(defs_begin(), defs_end());
    }
    Operand *defs_begin() {
        return &def;
    }
    Operand *defs_end() {
        return &def + hasDef();
    }

    Operand &getOp(size_t i) {
        return *(operands.begin() + i);
    }

    void addOp(Operand op) {
        operands.push_back(new Operand(op));
    }

    auto op() {
        return iter(op_begin(), op_end());
    }
    iterator op_begin() {
        return operands.begin();
    }
    iterator op_end() {
        return operands.end();
    }

    void dumpOp(std::ostream &os, Operand &op) {
        switch (op.kind) {
            case Operand::Nop:
                os << "none";
                break;
            case Operand::Reg:
                if (op.regOp.isVirReg()) {
                    os << "(vreg #" << op.regOp.getRegNo() << ")";
                } else {
                    os << "(reg #" << op.regOp.getRegNo() << ")";
                }
                break;
            case Operand::Imm:
                os << op.value.imm;
                break;
        }
    }

    void dump(std::ostream &os) {
        switch ((TargetOpcode) opcode) {
            case TargetAdd:
                os << "add";
                break;
            case TargetSub:
                os << "sub";
                break;
            case TargetMul:
                os << "mul";
                break;
            case TargetDiv:
                os << "div";
                break;
            case TargetMod:
                os << "mod";
                break;
            case TargetAnd:
                os << "and";
                break;
            case TargetOr:
                os << "or";
                break;
            case TargetXor:
                os << "xor";
                break;
            case TargetShl:
                os << "shl";
                break;
            case TargetShr:
                os << "shr";
                break;
            case TargetEq:
                os << "eq";
                break;
            case TargetNe:
                os << "ne";
                break;
            case TargetLt:
                os << "lt";
                break;
            case TargetGt:
                os << "gt";
                break;
            case TargetLe:
                os << "le";
                break;
            case TargetMove:
                os << "move";
                break;
            case TargetCBr:
                os << "cbr";
                break;
            case TargetBr:
                os << "br";
                break;
            case TargetCmp:
                os << "cmp";
                break;
            case TargetRet:
                os << "ret";
                break;
            case TargetNone:
                os << "none";
                break;
            default:
                os << "unknown";
                break;
        }
        os << " ";
        if (hasDef()) {
            dumpOp(os << "def:", def);
        }
        for (auto &op : op()) {
            if (hasDef() || &op != op_begin().getPointer()) {
                os << ", ";
            }
            dumpOp(os, op);
        }
    }

};

#endif //DRAGON_MACHINEINSTR_H
