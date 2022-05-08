//
// Created by Alex on 2022/3/24.
//

#ifndef DRAGON_MACHINEINSTR_H
#define DRAGON_MACHINEINSTR_H

#include <list>
#include "PatternNode.h"

class MachineBlock;
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
    static Operand label(MachineBlock *block) {
        Operand Op;
        Op.kind = Label;
        Op.value.addr = block;
        return Op;
    }
    static Operand slot(unsigned index) {
        Operand Op;
        Op.kind = Slot;
        Op.value.slot = index;
        return Op;
    }
public:
    enum Kind {
        Nop,
        Reg,
        Imm,
        Label,
        Slot,
    };

    ///< The kind of operand.
    Kind kind = Nop;

    ///< Tagged union
    union Content {
        int64_t imm;
        float fimm;
        double dimm;
        MachineBlock *addr;
        unsigned slot;
    } value;

    ///< Register
    Register regOp;

    Operand() {}
    Operand(Kind kind) : kind(kind) {}
    Operand(Kind kind, Register reg) : kind(kind), regOp(reg) {}

    Kind getKind() const {
        return kind;
    }
    bool isImm() const {
        return kind == Imm;
    }
    bool isLabel() const {
        return kind == Label;
    }
    bool isSlot() const {
        return kind == Slot;
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

    Register getReg() const {
        assert(isReg());
        return regOp;
    }

    MachineBlock *getLabel() const {
        assert(isLabel());
        return value.addr;
    }

    bool operator==(const Operand &rhs) const {
        if (kind != rhs.kind) {
            return false;
        }

        switch (kind) {
            case Nop:
                return true;
            case Reg:
                return regOp == rhs.regOp;
            case Imm:
                return value.imm == rhs.value.imm;
            case Label:
                return value.addr == rhs.value.addr;
            case Slot:
                return value.slot == rhs.value.slot;
        }
    }
    bool operator!=(const Operand &rhs) const {
        return !(*this == rhs);
    }

    // override =
    Operand &operator=(const Operand &rhs) {
        kind = rhs.kind;
        value = rhs.value;
        regOp = rhs.regOp;
        return *this;
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
    TargetNot,
    TargetNeg,
    TargetMove,
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
    Operand defOp;
    NodeList<Operand> operands;
    MachineInstr() {}
    MachineInstr(unsigned opcode) : opcode(opcode) {}

    unsigned getOpcode() const {
        return opcode;
    }

    bool hasDef() const {
        return defOp.kind != Operand::Nop;
    }
    void setDef(Operand d) {
        defOp = d;
    }

    Operand def() const {
        return defOp;
    }
    auto defs() {
        return iter(defs_begin(), defs_end());
    }
    Operand *defs_begin() {
        return &defOp;
    }
    Operand *defs_end() {
        return &defOp + hasDef();
    }

    auto defs() const {
        return iter(defs_begin(), defs_end());
    }
    const Operand *defs_begin() const {
        return &defOp;
    }
    const Operand *defs_end() const {
        return &defOp + hasDef();
    }

    Operand &getOp(size_t i) const {
        return *(operands.begin() + i);
    }

    void addOp(Operand op) {
        operands.push_back(new Operand(op));
    }

    auto ops() {
        return iter(op_begin(), op_end());
    }
    iterator op_begin() {
        return operands.begin();
    }
    iterator op_end() {
        return operands.end();
    }

    auto ops() const {
        return iter(op_begin(), op_end());
    }
    iterator op_begin() const {
        return operands.begin();
    }
    iterator op_end() const {
        return operands.end();
    }

    void dumpOp(std::ostream &os, Operand &op);

    void dump(std::ostream &os);

};

#endif //DRAGON_MACHINEINSTR_H
