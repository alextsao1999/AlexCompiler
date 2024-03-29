//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_INSTRUCTION_H
#define DRAGONIR_INSTRUCTION_H

#include <Value.h>
#include <Node.h>
#include <Type.h>
#include <memory>
#include "Context.h"
#include "SymbolTable.h"
#include "PatternNode.h"
enum BinaryOp {
    BinNone,
    Add = Pattern::Add,
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
    LastBinaryOp
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

    static inline std::unique_ptr<Use[]>
    CopyAndRemoveUse(std::unique_ptr<Use[]> &uses, size_t size, size_t i) {
        auto *UseElements = new Use[size - 1];
        std::uninitialized_copy(uses.get(), uses.get() + i, UseElements);
        std::uninitialized_copy(uses.get() + i + 1, uses.get() + size,
                                UseElements + i);
        return std::unique_ptr<Use[]>(UseElements);
    }
    static inline std::unique_ptr<Use[]>
    CopyAndAddUse(std::unique_ptr<Use[]> &uses, size_t size, Value *user, Value *value) {
        auto *UseElements = new Use[size + 1];
        std::uninitialized_copy(uses.get(), uses.get() + size, UseElements);
        new (&UseElements[size]) Use(user, value);
        return std::unique_ptr<Use[]>(UseElements);
    }
public:
    using iterator = IterWrapper<Use *, UseOpWrapper<Use, Value>>;
    using op_range = IterRange<iterator>;
public:
    Instruction() = delete;
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

    std::string nameForDebug;
    SymbolTable *getSymbolTable() const;
    const std::string &getName() {
        auto *ST = getSymbolTable();
        ASSERT(ST);
        return ST->getName(this);
    }
    void setName(StrView name) {
        auto *ST = getSymbolTable();
        ASSERT(ST);
        ST->setName(this, name);
        nameForDebug = name;
    }

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

    Context *getContext() const;
    Opcode getOpcode() const { return opcode; }
    inline size_t getOperandNum() const { return numOperands; }
    inline Value *getOperand(size_t i) const {
        ASSERT(i < numOperands);
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

    iterator op_begin() const { return iterator(getTrailingOperand()); }
    iterator op_end() const { return iterator(getTrailingOperand() + getOperandNum()); }
    op_range ops() { return op_range(op_begin(), op_end()); }

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
        auto *ST = getSymbolTable();
        ASSERT(ST);
        auto &Name = getName();
        if (Name.empty()) {
            os << "%" << ST->getCount(this);
        } else {
            os << "%" << Name << "." << ST->getCount(this);
        }
        return os;
    }

    Instruction *clone() const;
protected:
    inline Use *getTrailingOperand() const { return trailingOperands.get(); }
    inline Use *getUse(size_t i) const { return getTrailingOperand() + i; }

};

class AssignInst : public Instruction {
public:
    AssignInst(Value *lhs, Value *rhs) : Instruction(OpcodeAssign, {lhs, rhs}) {}
    AssignInst(const AssignInst &other) : Instruction(other) {}

    Value *getLHS() const { return getOperand(0); }
    Value *getRHS() const { return getOperand(1); }

    void dump(std::ostream &os) override {
        if (auto *LHS = getLHS()->as<Instruction>()) {
            LHS->dumpName(os);
        }
        os << " = " << getRHS()->dumpOperandToString();
    }
};

class OutputInst : public Instruction {
public:
    ///< The output type for this instruction
    Type *type = nullptr;
    OutputInst(Type *type, Opcode opcode) : Instruction(opcode, OpcodeNum[opcode]), type(type) {}
    OutputInst(Type *type, BasicBlock *parent, Opcode opcode) : Instruction(parent, opcode), type(type) {}
    OutputInst(Type *type, BasicBlock *parent, Opcode opcode, unsigned numOperands) : Instruction(opcode, numOperands), type(type) {}
    OutputInst(Type *type, Opcode opcode, const std::vector<Value *> &values) : Instruction(opcode, values),
                                                                                type(type) {}
    OutputInst(const OutputInst &other) : Instruction(other), type(other.type) {}
    using Instruction::Instruction;
    inline Type *getOutputType() const { return type; }
    void setType(Type *ty) { this->type = ty; }
    Type *getType() override {
        return getOutputType();
    }
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
        // We don't need to dump the size for now
        // os << ", " << allocatedSize;
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
    LoadInst(Type *ty) : OutputInst(ty, OpcodeLoad) {}
    LoadInst(Value *ptr) : OutputInst(ptr->getType(), OpcodeLoad, {ptr}) {}
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
    CopyInst(Type *ty) : OutputInst(ty, OpcodeCopy) {}
    CopyInst(Value *val) : OutputInst(val->getType(), OpcodeCopy, {val}) {}
    CopyInst(const CopyInst &other) : OutputInst(other) {}

    Value *getVal() const {
        return getOperand(0);
    }
};

class CastInst : public OutputInst {
public:
    CastInst(Type *type) : OutputInst(type, OpcodeCast) {}
    CastInst(const CastInst &other) : OutputInst(other) {}

    Value *getVal() const {
        return getOperand(0);
    }
};

class PhiInst : public OutputInst {
public:
    static PhiInst *Create(Type *ty, BasicBlock *bb, StrView name = "");
public:
    PhiInst(Type *ty) : OutputInst(ty, OpcodePhi) {}
    //PhiInst(size_t numOperands) : OutputInst(OpcodePhi, numOperands) {}
    PhiInst(const PhiInst &other);
    ~PhiInst() override = default;

    void fill(std::map<BasicBlock *, Value *> &values);
    BasicBlock *getIncomingBlock(Use &use) const;
    BasicBlock *getIncomingBlock(size_t i) const;
    Use *findIncomingUse(BasicBlock *bb) {
        for (auto I = 0; I < getOperandNum(); ++I) {
            if (getIncomingBlock(I) == bb) {
                return getUse(I);
            }
        }
        return nullptr;
    }
    Value *findIncomingValue(BasicBlock *bb) {
        for (auto I = 0; I < getOperandNum(); ++I) {
            if (getIncomingBlock(I) == bb) {
                return getOperand(I);
            }
        }
        return nullptr;
    }
    void removeIncoming(Use *use) {
        auto Idx = use - trailingOperands.get();
        ASSERT(Idx < getOperandNum());
        removeIncoming(Idx);
    }
    void removeIncoming(BasicBlock *bb) {
        for (auto I = 0; I < getOperandNum(); ++I) {
            if (getIncomingBlock(I) == bb) {
                removeIncoming(I);
            }
        }
    }
    void removeIncoming(size_t i) {
        incomingBlocks = CopyAndRemoveUse(incomingBlocks, getOperandNum(), i);
        trailingOperands = CopyAndRemoveUse(trailingOperands, getOperandNum(), i);
        numOperands--;
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
        ASSERT(getCalleeType());
        return getCalleeType()->getReturnType();
    }

    Type *getCalleeType() const;

    Function *getCallee() const {
        return callee;
    }

    void setCallee(Function *c) {
        callee = c;
    }

    void dump(std::ostream &os) override;

    unsigned getArgNum() const {
        return getOperandNum();
    }

    Value *getArg(unsigned i) const {
        return getOperand(i);
    }

    auto args() {
        return iter(op_begin(), op_end());
    }

};

class NotInst : public OutputInst {
public:
    NotInst(Type *ty) : OutputInst(ty, OpcodeNot) {}
    NotInst(Value *val) : OutputInst(val->getType(), OpcodeNot, {val}) {}
    NotInst(const NotInst &other) : OutputInst(other) {}

    Value *getVal() const {
        return getOperand(0);
    }
};

class NegInst : public OutputInst {
public:
    NegInst(Type *ty) : OutputInst(ty, OpcodeNeg) {}
    NegInst(Value *val) : OutputInst(val->getType(), OpcodeNeg, {val}) {}
    NegInst(const NegInst &other) : OutputInst(other) {}

    Value *getVal() const {
        return getOperand(0);
    }
};

class BinaryInst : public OutputInst {
    BinaryOp op;
public:
    inline static BinaryInst *Create(BinaryOp op, Value *lhs, Value *rhs) {
        auto *type = Type::getMaxType(lhs->getType(), rhs->getType());
        return new BinaryInst(type, op, lhs, rhs);
    }
public:
    BinaryInst(Type *ty) : OutputInst(ty, OpcodeBinary) {}
    BinaryInst(Type *ty, BinaryOp op, Value *lhs, Value *rhs) : OutputInst(ty, OpcodeBinary, {lhs, rhs}), op(op) {}
    BinaryInst(const BinaryInst &other) : OutputInst(other), op(other.op) {}

    BinaryOp getOp() {
        return op;
    }

    Type *getType() override {
        return getOutputType();
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
    GetPtrInst(Type *ty) : OutputInst(ty, OpcodeGetPtr) {}
    GetPtrInst(Value *base, Value *offset) : OutputInst(base->getType(), OpcodeGetPtr, {base, offset}) {}
    GetPtrInst(const GetPtrInst &other) : OutputInst(other) {}

    Type *getType() override {
        return getBase()->getType();
    }

    Value *getBase() {
        ASSERT(getOperandNum() > 0);
        return getOperand(0);
    }

    Value *getOffset() {
        ASSERT(getOperandNum() > 1);
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
        ASSERT(getOperandNum() == 1);
        return getOperand(0);
    }

};

template<typename SubTy, typename RetTy = void, typename...Args>
class InstVisitor {
public:
    RetTy visit(Value *value, Args&&...args) {
        if (value->isInstruction()) {
            return visit(value->cast<Instruction>(), std::forward<Args>(args)...);
        }
        return RetTy();
    }
    inline RetTy visit(Instruction &inst, Args&&...args) {
        return visit(&inst, std::forward<Args>(args)...);
    }
    RetTy visit(Instruction *inst, Args&&...args) {
        switch (inst->getOpcode()) {
#define DEFINE_OPCODE(NAME, c, CLASS) case Opcode##NAME: \
            return static_cast<SubTy *>(this)->visit##NAME(static_cast<CLASS *>(inst), std::forward<Args>(args)...);
            OPCODE_LIST(DEFINE_OPCODE)
#undef DEFINE_OPCODE
        default:
            ASSERT(false);
        }
    }
#define DEFINE_OPCODE(NAME, c, CLASS) virtual RetTy visit##NAME(CLASS *value, Args&&...args) { return RetTy(); }
    OPCODE_LIST(DEFINE_OPCODE);
#undef DEFINE_OPCODE
};

#endif //DRAGONIR_INSTRUCTION_H
