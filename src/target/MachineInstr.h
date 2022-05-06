//
// Created by Alex on 2022/3/24.
//

#ifndef DRAGON_MACHINEINSTR_H
#define DRAGON_MACHINEINSTR_H

#include <list>
#include "PatternNode.h"

class Operand {
public:
    enum Kind {
        Nop,
        VirReg,
        PhyReg,
        Imm,
    };
    Kind kind = Nop;
    union {
        int64_t imm;
        float fvalue;
        double dvalue;
        int virReg;
        int phyReg;
    } value;
    PatternNode *origin = nullptr;
    Operand() : kind(Nop), origin(nullptr) {}
    Operand(Kind kind, PatternNode *origin) : kind(kind), origin(origin) {}
    Kind getKind() const {
        return kind;
    }
    bool isVirReg() const {
        return kind == VirReg;
    }
    bool isPhyReg() const {
        return kind == PhyReg;
    }
    bool isReg() const {
        return isVirReg() || isPhyReg();
    }

    PatternNode *getOrigin() const {
        return origin;
    }

    static Operand from(PatternNode *node) {
        if (node->is<VirRegNode>()) {
            return Operand(VirReg, node);
        }
        if (node->is<PhyRegNode>()) {
            return Operand(PhyReg, node);
        }
        if (node->is<ConstantNode>()) {
            return Operand(Imm, node);
        }
        return Operand();
    }
    static Operand imm(PatternNode *node) {
        return Operand(Imm, node);
    }
    static Operand reg(PatternNode *node) {
        if (node->is<VirRegNode>()) {
            return Operand(VirReg, node);
        }
        if (node->is<PhyRegNode>()) {
            return Operand(PhyReg, node);
        }
        assert(false);
        return Operand(Nop, node);
    }
    static Operand imm(int64_t value) {
        Operand Op;
        Op.kind = Imm;
        Op.value.imm = value;
        return Op;
    }

};

class MachineBlock;
class MachineInstr : public NodeWithParent<MachineInstr, MachineBlock> {
public:
    using iterator = std::vector<std::unique_ptr<Operand>>::iterator;
    unsigned opcode = 0;
    Operand def;
    std::vector<std::unique_ptr<Operand>> operands;
    MachineInstr() {}
    MachineInstr(unsigned opcode) : opcode(opcode) {}

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
        return *operands[i];
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

};

#endif //DRAGON_MACHINEINSTR_H
