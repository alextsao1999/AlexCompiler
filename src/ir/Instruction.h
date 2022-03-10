//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_INSTRUCTION_H
#define DRAGONIR_INSTRUCTION_H

#include <Value.h>
#include <Node.h>
#include <Type.h>

#define OPCODE_LIST(S) \
S(Alloca, 1)           \
S(Phi, 0)              \
S(Undef, 0)            \
S(Br, 1)               \
S(CondBr, 3)           \
S(Binary, 2)           \
S(Ret, 1)              \
S(IntConst, 0)         \
S(StrConst, 0)         \
S(BoolConst, 0)        \
S(Load, 1)             \
S(Store, 2)

enum Opcode {
#define DEFINE_OPCODE(name, c) Opcode##name,
    OPCODE_LIST(DEFINE_OPCODE)
#undef DEFINE_OPCODE
};

static const size_t OpcodeNum[] = {
#define DEFINE_OPCODE(name, c) c,
        OPCODE_LIST(DEFINE_OPCODE)
#undef DEFINE_OPCODE
};

enum BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Shl,
    Shr,
    And,
    Or,
    Xor,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
};

class BasicBlock;

class Instruction : public NodeWithParent<Instruction, BasicBlock>, public Value {
    Opcode opcode;
    size_t numOperands;
    std::unique_ptr<Use[]> trailingOperands;
public:
    Instruction(Opcode opcode) : opcode(opcode), numOperands(OpcodeNum[opcode]) {}
    Instruction(Opcode opcode, size_t numTrailingOperands) : opcode(opcode), trailingOperands(new Use[numTrailingOperands]) {}
    Instruction(Opcode opcode, std::initializer_list<Value *> values);

    const std::string &getName();
    void setName(std::string_view name);

    inline bool isTerminator() const {
        switch (opcode) {
            case OpcodeBr:
            case OpcodeCondBr:
            case OpcodeRet:
                return true;
            default:
                return false;
        }
    }

    Opcode getOpcode() const { return opcode; }
    size_t getOperandNum() const { return numOperands; }

    Value *getOperand(size_t i) {
        return getTrailingOperand()[i].getUser();
    }

    void setOperand(size_t i, Use use) { getTrailingOperand()[i] = std::move(use); }

    auto operands() {
        return IterRange<Use *>(getTrailingOperand(), getTrailingOperand() + getOperandNum());
    }

    void dump(std::ostream &os, int level = 0) override;
    void dumpAsOperand(std::ostream &os) override {
        os << "%" << getName();
    }

protected:
    inline Use *getTrailingOperand() { return trailingOperands.get(); }
    inline const Use *getTrailingOperand() const { return trailingOperands.get(); }

};

class OutputInst : public Instruction {
public:
    using Instruction::Instruction;
    void dump(std::ostream &os, int level) override {
        dumpAsOperand(os);
        os << " = ";
        Instruction::dump(os, level);
    }

};

class AllocaInst : public OutputInst {
public:
    AllocaInst() : OutputInst(OpcodeAlloca) {}
    AllocaInst(Type *type) : OutputInst(OpcodeAlloca, {type}) {}

    Type *getType() override {
        return getOperand(0)->cast<Type>();
    }

};

class TerminatorInst : public Instruction {
public:
    using Instruction::Instruction;
};

class BranchInst : public TerminatorInst {
public:
    BranchInst() : TerminatorInst(OpcodeBr) {}
    BranchInst(BasicBlock *target);

    BasicBlock *getTarget() {
        assert(getOperand(0)->isa<BasicBlock>());
        return getOperand(0)->cast<BasicBlock>();
    }

};

class CondBrInst : public TerminatorInst {
public:
    CondBrInst() : TerminatorInst(OpcodeCondBr) {}
    CondBrInst(Value *cond, BasicBlock *trueTarget, BasicBlock *falseTarget);

    Value *getCond() {
        return getOperand(0);
    }

    BasicBlock *getTrueTarget() {
        assert(getOperand(1)->isa<BasicBlock>());
        return getOperand(1)->cast<BasicBlock>();
    }

    BasicBlock *getFalseTarget() {
        assert(getOperand(2)->isa<BasicBlock>());
        return getOperand(2)->cast<BasicBlock>();
    }

};

class LoadInst : public OutputInst {
public:
    LoadInst() : OutputInst(OpcodeLoad) {}
    LoadInst(Value *ptr) : OutputInst(OpcodeLoad, {ptr}) {}

    Value *getPtr() {
        return getOperand(0);
    }

};

class StoreInst : public Instruction {
public:
    StoreInst() : Instruction(OpcodeStore) {}
    StoreInst(Value *ptr, Value *val) : Instruction(OpcodeStore, {ptr, val}) {}

    Value *getPtr() {
        return getOperand(0);
    }

    Value *getVal() {
        return getOperand(1);
    }

};

class PhiInst : public OutputInst {
public:
    PhiInst() : OutputInst(OpcodePhi) {}
    PhiInst(size_t numOperands) : OutputInst(OpcodePhi, numOperands) {}

    void addIncoming(Value *val, BasicBlock *bb) {
        //getTrailingOperand()[getOperandNum() - 1] = Use(val, bb);

    }

};

class IntConstInst : public Instruction {
    int64_t value = 0;
public:
    IntConstInst() : Instruction(OpcodeIntConst) {}
    IntConstInst(int64_t val) : Instruction(OpcodeIntConst), value(val) {}

    void dumpAsOperand(std::ostream &os) override {
        os << value;
    }

    int64_t getVal() {
        return value;
    }

};

class StrConstInst : public Instruction {
    std::string value;
public:
    StrConstInst() : Instruction(OpcodeStrConst) {}
    StrConstInst(std::string val) : Instruction(OpcodeStrConst), value(val) {}

    void dumpAsOperand(std::ostream &os) override {
        os << value;
    }

    std::string getVal() {
        return value;
    }
};

class BoolConstInst : public Instruction {
    bool value = false;
public:
    BoolConstInst() : Instruction(OpcodeBoolConst) {}
    BoolConstInst(bool val) : Instruction(OpcodeBoolConst), value(val) {}

    void dumpAsOperand(std::ostream &os) override {
        os << value;
    }

    bool getVal() {
        return value;
    }
};

class BinaryInst : public OutputInst {
    BinaryOp op;
public:
    BinaryInst() : OutputInst(OpcodeBinary) {}
    BinaryInst(BinaryOp op, Value *lhs, Value *rhs) : OutputInst(OpcodeBinary, {lhs, rhs}), op(op) {}

    BinaryOp getOp() {
        return op;
    }

    Value *getLHS() {
        return getOperand(0);
    }

    Value *getRHS() {
        return getOperand(1);
    }

};

class RetInst : public TerminatorInst {
public:
    RetInst() : TerminatorInst(OpcodeRet) {}
    RetInst(Value *val) : TerminatorInst(OpcodeRet, {val}) {}

    Value *getRetVal() {
        return getOperand(0);
    }

};

#endif //DRAGONIR_INSTRUCTION_H
