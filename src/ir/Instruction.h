//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_INSTRUCTION_H
#define DRAGONIR_INSTRUCTION_H

#include <Value.h>
#include <Node.h>
#include <Type.h>
#include <memory>

enum BinaryOp {
    None,
    Add,
    Sub,
    Mul,
    Div,
    Rem,
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

inline bool isCommutative(BinaryOp op) {
    switch (op) {
        case Add:
        case Mul:
        case And:
        case Or:
        case Xor:
            return true;
        default:
            return false;
    }
}

class Function;
class BasicBlock;

/**
 * Undef is the undefined value.
 * We don't use nullptr to avoid confusion.
 */
class Undef : public Value {
public:
    void dumpAsOperand(std::ostream &os) override {
        os << "undef";
    }
};

/**
 * Param is the parameter value.
 */
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
        type->dump(os);
        os << " %" << name;
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

/**
 * Constant is the base class of all constant values.
 */
class Constant : public Value {
protected:
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

    Ty &getVal() {
        return val;
    }

    const Ty &getVal() const {
        return val;
    }

    void dumpAsOperand(std::ostream &os) override {
        type->dump(os);
        os << " " << val;
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

    // Allocate Uses and construct the trailingOperands array.
    static inline std::unique_ptr<Use[]>
    AllocateUses(Instruction *user, const std::vector<Value *> &values) {
        auto *UseElements = new Use[values.size()];
        for (auto I = 0; I < values.size(); I++) {
            new(&UseElements[I]) Use(user, values[I]);
        }
        return std::unique_ptr<Use[]>(UseElements);
    }

    static inline std::unique_ptr<Use[]>
    AllocateUses(Instruction *user, size_t numValues) {
        auto *UseElements = new Use[numValues];
        for (auto I = 0; I < numValues; I++) {
            new(&UseElements[I]) Use(user);
        }
        return std::unique_ptr<Use[]>(UseElements);
    }
public:
    Instruction() {}
    Instruction(BasicBlock *parent, Opcode opcode);
    Instruction(Opcode opcode) : Instruction(opcode, OpcodeNum[opcode]) {}
    Instruction(Opcode opcode, const std::vector<Value *> &values) : opcode(opcode), numOperands(values.size()),
                                                                        trailingOperands(AllocateUses(this, values)) {}
    Instruction(Opcode opcode, size_t numTrailingOperands) : opcode(opcode), numOperands(numTrailingOperands),
                                                             trailingOperands(AllocateUses(this, numTrailingOperands)) {}
    Instruction(const Instruction &other) : Instruction(other.opcode, other.numOperands) {
        // copy trailing operands
        for (size_t I = 0; I < numOperands; I++) {
            setOperand(I, other.getOperand(I));
        }
    }

    const std::string &getName();
    void setName(std::string_view name);

    inline bool hasSideEffects() const {
        switch (opcode) {
            case OpcodeRet:
            case OpcodeBr:
            case OpcodeCall:
            case OpcodeCondBr:
            case OpcodeLoad:
            case OpcodeStore:
                return true;
            default:
                return false;
        }
    }

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
        assert(i < numOperands);
        return getTrailingOperand()[i].getValue();
    }

    void setOperand(size_t i, Value *value) { getTrailingOperand()[i].set(value); }
    void setOperands(const std::vector<Value *> &values) {
        numOperands = values.size();
        trailingOperands = AllocateUses(this, values);
    }

    inline auto begin() {
        return getTrailingOperand();
    }

    inline auto end() {
        return getTrailingOperand() + getOperandNum();
    }

    IterRange<Use *> operands() const {
        return iter(getTrailingOperand(), getTrailingOperand() + getOperandNum());
    }

    virtual unsigned getNumSuccessors() const { return 0; }
    virtual BasicBlock *getSuccessor(unsigned i) const { return nullptr; }
    virtual void setSuccessor(unsigned i, BasicBlock *bb) {}

    void dump(std::ostream &os) override;
    void dumpAsOperand(std::ostream &os) override {
        if (auto *Ty = getType()) {
            Ty->dump(os);
            os << " ";
        }
        dumpName(os);
    }
    inline std::ostream &dumpName(std::ostream &os) {
        os << "%" << getName();
        return os;
    }

    Instruction *clone() const;
protected:
    inline Use *getTrailingOperand() const { return trailingOperands.get(); }
    inline Use *getUse(size_t i) const { return getTrailingOperand() + i; }

};

class OutputInst : public Instruction {
public:
    using Instruction::Instruction;
    void dump(std::ostream &os) override {
        dumpName(os) << " = ";
        Instruction::dump(os);
    }
};

class AllocaInst : public Instruction {
    Type *allocatedType;
    unsigned allocatedSize = 1;
public:
    AllocaInst(const AllocaInst &other) : Instruction(other), allocatedType(other.allocatedType),
                                          allocatedSize(other.allocatedSize) {}
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

    void dump(std::ostream &os) override {
        dumpName(os) << " = alloca ";
        allocatedType->dump(os);
        os << ", " << allocatedSize;
    }
};

class StoreInst : public Instruction {
public:
    StoreInst() : Instruction(OpcodeStore) {}
    StoreInst(Value *ptr, Value *val) : Instruction(OpcodeStore, {ptr, val}) {}
    StoreInst(const StoreInst &other) : Instruction(other) {}

    Value *getPtr() const {
        return getOperand(0);
    }

    Value *getVal() const {
        return getOperand(1);
    }

};

class LoadInst : public OutputInst {
public:
    LoadInst() : OutputInst(OpcodeLoad) {}
    LoadInst(Value *ptr) : OutputInst(OpcodeLoad, {ptr}) {}
    LoadInst(const LoadInst &other) : OutputInst(other) {}

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

class CopyInst : public OutputInst {
public:
    CopyInst() : OutputInst(OpcodeCopy) {}
    CopyInst(Value *val) : OutputInst(OpcodeCopy, {val}) {}
    CopyInst(const CopyInst &other) : OutputInst(other) {}

    Type *getType() override {
        assert(getVal());
        return getVal()->getType();
    }

    Value *getVal() const {
        return getOperand(0);
    }
};

class CastInst : public OutputInst {
    Type *targetType;
public:
    CastInst() : OutputInst(OpcodeCast) {}
    CastInst(const CastInst &other) : OutputInst(other), targetType(other.targetType) {}
    CastInst(Type *type) : OutputInst(OpcodeCast), targetType(type) {}

    Type *getType() override {
        return targetType;
    }

    Type *getTargetType() const {
        return targetType;
    }

    Value *getVal() const {
        return getOperand(0);
    }
};

class PhiInst : public OutputInst {
public:
    static PhiInst *Create(BasicBlock *bb, StrView name = "");
public:
    PhiInst() : OutputInst(OpcodePhi) {}
    PhiInst(size_t numOperands) : OutputInst(OpcodePhi, numOperands) {}
    PhiInst(const PhiInst &other);
    ~PhiInst() override = default;

    Type *getType() override {
        assert(numOperands > 0 && getOperand(0));
        return getOperand(0)->getType();
    }

    void fill(std::map<BasicBlock *, Value *> &values);
    BasicBlock *getIncomingBlock(Use &use) const;
    BasicBlock *getIncomingBlock(size_t i) const;
    Use *findIncoming(BasicBlock *bb) {
        for (auto I = 0; I < getOperandNum(); ++I) {
            if (getIncomingBlock(I) == bb) {
                return getUse(I);
            }
        }
        return nullptr;
    }

    void setIncomingBlock(size_t i, BasicBlock *bb);
    // fill incomings with vector of bb and value
    void setIncomings(std::vector<std::pair<Value *, BasicBlock *>> incomings) {
        numOperands = incomings.size();
        trailingOperands = AllocateUses(this, incomings.size());
        incomingBlocks = AllocateUses(this, incomings.size());
        size_t I = 0;
        for (auto &[val, bb] : incomings) {
            setOperand(I, val);
            setIncomingBlock(I, bb);
        }
    }

    // incomings iterator
    IterRange<Use *> incomings() const {
        return iter(incomingBlocks.get(), incomingBlocks.get() + getOperandNum());
    }

    void dump(std::ostream &os) override;

private:
    std::unique_ptr<Use[]> incomingBlocks;

};

class CallInst : public OutputInst {
    Function *callee = nullptr;
public:
    CallInst(Function *callee) : OutputInst(OpcodeCall), callee(callee) {}
    CallInst(Function *callee, const std::vector<Value *> &args) : OutputInst(OpcodeCall, args), callee(callee) {}
    CallInst(const CallInst &other) : OutputInst(other), callee(other.callee) {}

    Type *getType() override {
        return getReturnType();
    }

    Type *getReturnType() const {
        assert(getCalleeType());
        return getCalleeType()->getReturnType();
    }

    Type *getCalleeType() const;

    Function *getCallee() const {
        return callee;
    }

    void dump(std::ostream &os) override;

    Value *getArg(unsigned i) const {
        return getOperand(i);
    }

    auto args() {
        return iter(getTrailingOperand(), getTrailingOperand() + getOperandNum());
    }

};

class UnaryInst : public OutputInst {
public:

};

class BinaryInst : public OutputInst {
    BinaryOp op;
public:
    BinaryInst() : OutputInst(OpcodeBinary) {}
    BinaryInst(BinaryOp op, Value *lhs, Value *rhs) : OutputInst(OpcodeBinary, {lhs, rhs}), op(op) {}
    BinaryInst(const BinaryInst &other) : OutputInst(other), op(other.op) {}

    BinaryOp getOp() {
        return op;
    }

    Type *getType() override {
        return Type::getMaxType(getLHS()->getType(), getRHS()->getType());
    }

    Value *getLHS() {
        return getOperand(0);
    }

    Value *getRHS() {
        return getOperand(1);
    }

};

class GetPtrInst : public OutputInst {
public:
    GetPtrInst() : OutputInst(OpcodeGetPtr) {}
    GetPtrInst(Value *base, Value *offset) : OutputInst(OpcodeGetPtr, {base, offset}) {}
    GetPtrInst(const GetPtrInst &other) : OutputInst(other) {}

    Type *getType() override {
        return getBase()->getType();
    }

    Value *getBase() {
        assert(getOperandNum() > 0);
        return getOperand(0);
    }

    Value *getOffset() {
        assert(getOperandNum() > 1);
        return getOperand(1);
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
    BranchInst(const BranchInst &other) : TerminatorInst(other) {}

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
    CondBrInst(const CondBrInst &other) : TerminatorInst(other) {}

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

class RetInst : public TerminatorInst {
public:
    RetInst() : TerminatorInst(OpcodeRet) {}
    RetInst(Value *val) : TerminatorInst(OpcodeRet, {val}) {}
    RetInst(const RetInst &other) : TerminatorInst(other) {}

    bool isVoidRet() const {
        return getOperandNum() == 0;
    }

    Value *getRetVal() const {
        assert(getOperandNum() == 1);
        return getOperand(0);
    }

};

#endif //DRAGONIR_INSTRUCTION_H
