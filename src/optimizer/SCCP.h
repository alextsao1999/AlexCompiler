//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_SCCP_H
#define DRAGONIR_SCCP_H
#include "Function.h"
#include "PassManager.h"
#include "ConstantFolder.h"
struct LatticeValue {
    enum LatticeType {
        Undef,
        Const,
        NaC, // Not a constant
    };

    LatticeType type = Undef;
    Value *value = nullptr;
    LatticeValue() {}
    LatticeValue(LatticeType type, Value *value) : type(type), value(value) {}

    inline static LatticeValue getNaC() { return {NaC, nullptr}; }
    inline static LatticeValue getConstant(Value *op) { return {Const, op}; }

    bool is(LatticeType ty) {
        return type == ty;
    }

    bool isConstant() {
        return type == Const;
    }

    bool isNaC() {
        return type == NaC;
    }

    bool isUndef() {
        return type == Undef;
    }

    // meet操作符
    LatticeValue operator^(const LatticeValue &rhs) {
        // 返回定义了常量的Lattice Value
        if (type == Undef) return rhs;
        if (rhs.type == Undef) return *this;

        if (type == NaC || rhs.type == NaC) {
            return getNaC();
        }
        assert(type == Const && rhs.type == Const);
        if (value == rhs.value) {
            return *this;
        }
        return getNaC();
    }

    LatticeValue &operator^=(const LatticeValue &rhs) {
        *this = *this ^ rhs;
        return *this;
    }

    bool operator==(const LatticeValue &rhs) {
        return std::tie(type, value) == std::tie(rhs.type, rhs.value);
    }

    bool operator!=(const LatticeValue& rhs) {
        return !operator==(rhs);
    }

    inline Value *getValue() const {
        assert(type == Const);
        return value;
    }

};

class SCCPFunction {
    Function *fun;
    std::vector<BasicBlock *> cfgWorklist;
    std::vector<Instruction *> ssaWorklist;
    std::map<BasicBlock *, bool> mapBBExcuted;
    std::map<Value *, LatticeValue> mapValToLatVal;
    LatticeValue returnVal;
public:
    SCCPFunction(Function *f) : fun(f) {}
    LatticeValue getLatticeVal(Value *val) {
        if (!val) {
            return LatticeValue::getNaC();
        }
        if (auto *Const = val->as<Constant>()) {
            return LatticeValue::getConstant(Const);
        }
        return mapValToLatVal[val];
    }
    void setLatticeVal(Instruction *val, LatticeValue latVal) {
        auto &Slot = mapValToLatVal[val];
        if (Slot != latVal) {
            Slot = latVal;
            ssaWorklist.push_back(val);
        }
    }
    bool isExecutable(BasicBlock *bb) {
        auto Iter = mapBBExcuted.find(bb);
        if (Iter == mapBBExcuted.end()) {
            return false;
        }
        return Iter->second;
    }
    bool isLatValTrue(LatticeValue val) {
        assert(val.isConstant());
        if (auto *Val = val.value->as<IntConstant>()) {
            return Val->getVal();
        }
        return false;
    }

    void evalOnInstruction(Instruction *instr) {
        switch (instr->getOpcode()) {
            default: unreachable();
            case OpcodeBr: {
                auto *Br = instr->cast<BranchInst>();
                cfgWorklist.push_back(Br->getTarget());
                break;
            }
            case OpcodeCondBr: {
                auto *Br = instr->cast<CondBrInst>();
                auto Val = getLatticeVal(Br->getCond());
                if (Val.isConstant()) {
                    cfgWorklist.push_back(isLatValTrue(Val) ? Br->getTrueTarget() : Br->getFalseTarget());
                } else {
                    cfgWorklist.push_back(Br->getTrueTarget());
                    cfgWorklist.push_back(Br->getFalseTarget());
                }
                break;
            }
            case OpcodePhi: {
                evalOnPhiInst(instr->cast<PhiInst>());
                break;
            }
            case OpcodeBinary: {
                evalOnBinary(instr->cast<BinaryInst>());
                break;
            }
            case OpcodeCall: {
                evalOnCall(instr->cast<CallInst>());
                break;
            }
            case OpcodeRet: {
                evalOnRet(instr->cast<RetInst>());
                break;
            }
            case OpcodeAlloca:
            case OpcodeCast:
            case OpcodeCopy:
            case OpcodeUnary:
            case OpcodeGetPtr:
            case OpcodeLoad:
            case OpcodeStore:
                break;
        }
    }
    void evalOnPhiInst(PhiInst *phi) {
        LatticeValue NewVal;
        for (auto I = 0; I < phi->getOperandNum(); ++I) {
            auto *BB = phi->getIncomingBlock(I);
            if (isExecutable(BB)) {
                NewVal ^= getLatticeVal(phi->getOperand(I));
            }
        }
        setLatticeVal(phi, NewVal);
    }
    void evalOnBinary(BinaryInst *bin) {
        auto LHS = getLatticeVal(bin->getLHS());
        auto RHS = getLatticeVal(bin->getRHS());
        LatticeValue Val = LatticeValue::getNaC();

        if (LHS.isConstant() && RHS.isConstant()) {
            if (auto *Cons = ConstantFolder::foldBin(bin->getOp(), LHS.value, RHS.value)) {
                Val = LatticeValue::getConstant(Cons);
            }
        }

        if (LHS.isNaC() && RHS.isConstant()) {
            if (auto *Int = RHS.getValue()->as<IntConstant>()) {
                if (Int->getVal() == 0 && bin->getOp() == BinaryOp::Mul) {
                    auto *Context = bin->getType()->getContext();
                    Val = LatticeValue::getConstant(Context->getInt(0));
                }
            }
        }

        if (LHS.isConstant() && RHS.isNaC()) {
            if (auto *Int = LHS.getValue()->as<IntConstant>()) {
                if (Int->getVal() == 0 && bin->getOp() == BinaryOp::Mul) {
                    auto *Context = bin->getType()->getContext();
                    Val = LatticeValue::getConstant(Context->getInt(0));
                }
            }
        }

        assert(!LHS.isUndef() && !RHS.isUndef());

        setLatticeVal(bin, Val);

    }
    void evalOnCall(CallInst *call) {
        SCCPFunction Func(call->getCallee());
        for (auto I = 0; I < call->getOperandNum(); ++I) {
            auto *Val = call->getOperand(I);
            Func.mapLatticeVal(Val, getLatticeVal(Val));
        }
        Func.runOnEntry();

    }
    void evalOnRet(RetInst *ret) {
        returnVal = getLatticeVal(ret->getOperand(0));
    }
    void mapLatticeVal(Value *val, LatticeValue latVal) {
        mapValToLatVal[val] = latVal;
    }

    void runOnEntry() {
        run(fun->getEntryBlock());
    }
    void run(BasicBlock *entry) {
        cfgWorklist.push_back(entry);
        while (!cfgWorklist.empty() || !ssaWorklist.empty()) {
            if (!cfgWorklist.empty()) {
                auto *Cur = cfgWorklist.back();
                cfgWorklist.pop_back();
                if (!mapBBExcuted[Cur]) {
                    mapBBExcuted[Cur] = true;
                    for (auto &Instr: Cur->getSubList()) {
                        evalOnInstruction(&Instr);
                    }
                }
            }
            if (!ssaWorklist.empty()) {
                auto *Cur = ssaWorklist.back();
                ssaWorklist.pop_back();
                // 判断所在的基本块是否可达
                if (mapBBExcuted[Cur->getParent()]) {
                    evalOnInstruction(Cur);
                }
            }
        }
    }

};

class SCCP : public FunctionPass {
public:
    void runOnFunction(Function &function) override {
        SCCPFunction Func(&function);
        Func.runOnEntry();

    }
};

#endif //DRAGONIR_SCCP_H
