//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_INSTRUCTION_H
#define DRAGONIR_INSTRUCTION_H

#include <Value.h>
#include <Node.h>
#include <Type.h>

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

class Function;
class BasicBlock;

class Undef : public Value {
public:
    void dumpAsOperand(std::ostream &os) override {
        os << "undef";
    }
};

class Param : public Value {
    std::string name;
    Type *type;
public:
    Param() : type(nullptr) {}
    Param(std::string_view name, Type *type) : name(name), type(type) {
        incRef();
    }

    std::string &getName() {
        return name;
    }

    Type *getType() override {
        return type;
    }

    void dumpAsOperand(std::ostream &os) override {
        std::cout << "%" << name;
    }

};

class Global : public Value {
    std::string name;
    Type *type;
public:
    Global(const std::string &name, Type *type) : name(name), type(type) {}

    const std::string &getName() const {
        return name;
    }

    Type *getType() const {
        return type;
    }

    void dumpAsOperand(std::ostream &os) override {
        os << "%" << name;
    }

};

class Constant : public Value {
    Type *type;
public:
    Constant(Type *type) : type(type) {
        incRef();
    }

    Type *getType() override {
        return type;
    }

};

template<typename Ty>
class ConstantVal : public Constant {
public:
    ConstantVal(Type *type, const Ty &val) : Constant(type), val(val) {
    }

    Ty &getVal() const {
        return val;
    }

    void dumpAsOperand(std::ostream &os) override {
        os << val;
    }

private:
    Ty val;
};
using IntConstant = ConstantVal<int64_t>;
using StrConstant = ConstantVal<std::string>;
using BoolConstant = ConstantVal<bool>;

class Instruction : public Value, public NodeWithParent<Instruction, BasicBlock> {
protected:
    Opcode opcode;
    size_t numOperands;
    std::unique_ptr<Use[]> trailingOperands;
public:
    Instruction() {}
    Instruction(BasicBlock *parent, Opcode opcode);
    Instruction(Opcode opcode) : opcode(opcode), numOperands(OpcodeNum[opcode]) {}
    Instruction(Opcode opcode, std::initializer_list<Value *> values) : opcode(opcode), numOperands(values.size()),
                                                                        trailingOperands(new Use[values.size()]) {
        auto *Operand = getTrailingOperand();
        for (auto &Value: values) {
            new(Operand++) Use(this, Value);
        }
    }

    Instruction(Opcode opcode, size_t numTrailingOperands) : opcode(opcode), numOperands(numTrailingOperands) {
        // allocate memory for trailing operands
        trailingOperands = std::unique_ptr<Use[]>(new Use[numTrailingOperands]);
        // construct the trailing operands
        for (size_t I = 0; I < numTrailingOperands; I++) {
            new(&trailingOperands[I]) Use(this);
        }
    }

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

    Value *getOperand(size_t i) const {
        return getTrailingOperand()[i].getValue();
    }

    void setOperand(size_t i, Value *value) { getTrailingOperand()[i].set(value); }

    auto operands() {
        return IterRange<Use *>(getTrailingOperand(), getTrailingOperand() + getOperandNum());
    }

    virtual unsigned getNumSuccessors() const { return 0; }
    virtual BasicBlock *getSuccessor(unsigned i) const { return nullptr; }
    virtual void setSuccessor(unsigned i, BasicBlock *bb) {}

    void dump(std::ostream &os, int level = 0) override;
    void dumpAsOperand(std::ostream &os) override {
        if (auto *Ty = getType()) {
            Ty->dump(os);
            os << " ";
        }
        os << "%" << getName();
    }

protected:
    inline Use *getTrailingOperand() const { return trailingOperands.get(); }

};

class OutputInst : public Instruction {
public:
    using Instruction::Instruction;
    void dump(std::ostream &os, int level) override {
        os << "%" << getName() << " = ";
        Instruction::dump(os, level);
    }

};

class AllocaInst : public Instruction {
    Type *allocatedType;
    unsigned allocatedSize = 1;

public:
    AllocaInst(Type *type) : Instruction(OpcodeAlloca), allocatedType(type) {}
    AllocaInst(Type *type, unsigned size) : Instruction(OpcodeAlloca), allocatedType(type), allocatedSize(size) {}

    Type *getType() override {
        return allocatedType->getPointerType();
    }

    inline Type *getAllocatedType() const {
        return allocatedType;
    }

    inline unsigned getAllocatedSize() const {
        return allocatedSize;
    }

    void dump(std::ostream &os, int level) override {
        os << "%" << getName() << " = alloca ";
        allocatedType->dump(os);
        os << ", " << allocatedSize;
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

    BasicBlock *getTarget() const;
    void setTarget(BasicBlock *target);

    unsigned int getNumSuccessors() const override {
        return 1;
    }

    BasicBlock *getSuccessor(unsigned int i) const override {
        return getTarget();
    }

    void setSuccessor(unsigned int i, BasicBlock *bb) override {
        setTarget(bb);
    }
};

class CondBrInst : public TerminatorInst {
public:
    CondBrInst() : TerminatorInst(OpcodeCondBr) {}
    CondBrInst(Value *cond, BasicBlock *trueTarget, BasicBlock *falseTarget);

    Value *getCond() const {
        return getOperand(0);
    }

    BasicBlock *getTrueTarget() const;

    BasicBlock *getFalseTarget() const;

    void setTrueTarget(BasicBlock *bb);

    void setFalseTarget(BasicBlock *bb);

    unsigned int getNumSuccessors() const override {
        return 2;
    }

    BasicBlock *getSuccessor(unsigned int i) const override {
        return i == 0 ? getTrueTarget() : getFalseTarget();
    }

    void setSuccessor(unsigned int i, BasicBlock *bb) override {
        if (i == 0) {
            setTrueTarget(bb);
        } else {
            setFalseTarget(bb);
        }
    }

};

class CallInst : public OutputInst {
    Function *callee = nullptr;
public:

    CallInst(Function *callee) : OutputInst(OpcodeCall), callee(callee) {}
    CallInst(Function *callee, std::initializer_list<Value *> args) : OutputInst(OpcodeCall, args) {}

    Function *getCallee() {
        return callee;
    }

    auto args() {
        return iter(getTrailingOperand(), getTrailingOperand() + getOperandNum());
    }

};

class LoadInst : public OutputInst {
public:
    LoadInst() : OutputInst(OpcodeLoad) {}
    LoadInst(Value *ptr) : OutputInst(OpcodeLoad, {ptr}) {}

    Value *getPtr() const {
        return getOperand(0);
    }

    Type *getType() override {
        if (auto *LoadTy = getOperand(0)->getType()) {
            if (LoadTy->isPointerType()) {
                return LoadTy->getPointerElementType();
            }
        }
        return nullptr;
    }

};

class StoreInst : public Instruction {
public:
    StoreInst() : Instruction(OpcodeStore) {}
    StoreInst(Value *ptr, Value *val) : Instruction(OpcodeStore, {ptr, val}) {}

    Value *getPtr() const {
        return getOperand(0);
    }

    Value *getVal() const {
        return getOperand(1);
    }

};

class PhiInst : public OutputInst {
public:
    static PhiInst *Create(BasicBlock *bb);
public:
    PhiInst() : OutputInst(OpcodePhi) {}
    PhiInst(size_t numOperands) : OutputInst(OpcodePhi, numOperands) {}
    ~PhiInst() override = default;

    void fill(std::map<BasicBlock *, Value *> &values);
    BasicBlock *getIncomingBlock(Use &use);
    BasicBlock *getIncomingBlock(size_t i);

    void dump(std::ostream &os, int level) override;

private:
    std::unique_ptr<Use[]> incomingBlocks;

};

class UnaryInst : public Instruction {
public:

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

    Value *getRetVal() const {
        return getOperand(0);
    }

};

#endif //DRAGONIR_INSTRUCTION_H
