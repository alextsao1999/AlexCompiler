//
// Created by Alex on 2022/3/8.
//
#include <algorithm>
#include "Instruction.h"
#include "BasicBlock.h"
#include "Function.h"

void Instruction::dump(std::ostream &os) {
    // dump opcode
    if (opcode == OpcodeBinary) {
        auto *Bin = this->cast<BinaryInst>();
        // dump op
        switch (Bin->getOp()) {
            case BinaryOp::Add:
                os << "add";
                break;
            case BinaryOp::Sub:
                os << "sub";
                break;
            case BinaryOp::Mul:
                os << "mul";
                break;
            case BinaryOp::Div:
                os << "div";
                break;
            case BinaryOp::Mod:
                os << "mod";
                break;
            case BinaryOp::Shl:
                os << "shl";
                break;
            case BinaryOp::Shr:
                os << "shr";
                break;
            case BinaryOp::And:
                os << "and";
                break;
            case BinaryOp::Or:
                os << "or";
                break;
            case BinaryOp::Xor:
                os << "xor";
                break;
            case BinaryOp::Eq:
                os << "eq";
                break;
            case BinaryOp::Ne:
                os << "ne";
                break;
            case BinaryOp::Lt:
                os << "lt";
                break;
            case BinaryOp::Le:
                os << "le";
                break;
            case BinaryOp::Gt:
                os << "gt";
                break;
            case BinaryOp::Ge:
                os << "ge";
                break;
            default:
                os << "???";
                break;
        }
    } else {
        std::string opcodeStr;
        switch (opcode) {
            default: unreachable();
#define OPCODE(NAME, COUNT, CLASS) \
        case Opcode##NAME: \
            opcodeStr = #NAME; \
            break;
            OPCODE_LIST(OPCODE)
#undef OPCODE
        }
        // lowercase of opcodeStr
        std::transform(opcodeStr.begin(), opcodeStr.end(), opcodeStr.begin(), ::tolower);
        os << opcodeStr;
    }

    os << " ";

    /*DUMP_REF(os, operands(), V, {
        V->dumpAsOperand(os);
    });*/

    os << dump_str(operands());

}

SymbolTable *Instruction::getSymbolTable() const {
    if (auto *P = getParent()) {
        if (auto *F = P->getParent()) {
            return &(F->getSymbolTable());
        }
    }
    return nullptr;
}

Instruction::Instruction(BasicBlock *parent, Opcode opcode) : Instruction(opcode, OpcodeNum[opcode]) {
    parent->append(this);
}

BranchInst::BranchInst(BasicBlock *target) : TerminatorInst(OpcodeBr, {target}) {

}

BasicBlock *BranchInst::getTarget() const {
    assert(getOperand(0)->isa<BasicBlock>());
    return getOperand(0)->cast<BasicBlock>();
}

void BranchInst::setTarget(BasicBlock *target) {
    setOperand(0, target);
}

CondBrInst::CondBrInst(Value *cond, BasicBlock *trueTarget, BasicBlock *falseTarget) : TerminatorInst(OpcodeCondBr,
                                                                                                      {cond, trueTarget,
                                                                                                       falseTarget}) {}

BasicBlock *CondBrInst::getTrueTarget() const {
    assert(getOperand(1)->isa<BasicBlock>());
    return getOperand(1)->cast<BasicBlock>();
}

BasicBlock *CondBrInst::getFalseTarget() const {
    assert(getOperand(2)->isa<BasicBlock>());
    return getOperand(2)->cast<BasicBlock>();
}

void CondBrInst::setTrueTarget(BasicBlock *target) {
    setOperand(1, target);
}

void CondBrInst::setFalseTarget(BasicBlock *target) {
    setOperand(2, target);
}

PhiInst *PhiInst::Create(Type *type, BasicBlock *bb, StrView name) {
    auto *Phi = new PhiInst(type);
    bb->addPhi(Phi);
    Phi->setName(name);
    return Phi;
}

PhiInst::PhiInst(const PhiInst &other) : OutputInst(other) {
    incomingBlocks = std::unique_ptr<Use[]>(new Use[getOperandNum()]);
    for (unsigned I = 0; I < getOperandNum(); ++I) {
        new(&incomingBlocks[I]) Use(this, other.getIncomingBlock(I));
    }
}

void PhiInst::fill(std::map<BasicBlock *, Value *> &values) {
    numOperands = values.size();
    trailingOperands = std::unique_ptr<Use[]>(new Use[numOperands]);
    incomingBlocks = std::unique_ptr<Use[]>(new Use[numOperands]);

    int I = 0;
    for (auto &[block, value] : values) {
        new(&incomingBlocks[I]) Use(this, block);
        new(&trailingOperands[I]) Use(this, value);
        I++;
    }

    // make sure all the types of incoming values are the same
    assert(numOperands >= 1);

    /**
     * FIXME: make sure all the types of incoming values are the same
     * But we don't have a way to get the type of a value that may be a phi node
     * or any other value what we don't have type of.
     */

    /*assert(std::all_of(begin(), end(), [&](auto &arg) {
        return arg.getValue()->getType() == begin()->getValue()->getType();
    }));*/

}

BasicBlock *PhiInst::getIncomingBlock(Use &use) const {
    auto Index = &use - getTrailingOperand();
    assert(Index >= 0 && Index < numOperands);
    return incomingBlocks[Index]->cast<BasicBlock>();
}

BasicBlock *PhiInst::getIncomingBlock(size_t i) const {
    assert(i < numOperands);
    return incomingBlocks[i]->cast<BasicBlock>();
}

void PhiInst::setIncomingBlock(size_t i, BasicBlock *bb) {
    assert(i < numOperands);
    incomingBlocks[i].set(bb);
}

void PhiInst::dump(std::ostream &os) {
    dumpName(os) << " = phi ";
    // FIXME: maybe we should dump the type here?
    os << dump_str(operands(), [this](Use &use) {
        auto *BB = getIncomingBlock(use);
        if (BB == nullptr) {
            return use.getValue() ? "[null: " + use->dumpOperandToString() + "]" : "[null: null]";
        }
        return "[" + BB->dumpOperandToString() +": " + use->dumpOperandToString() + "]";
    });
}

Instruction *Instruction::clone() const {
    switch (getOpcode()) {
        default: unreachable();
#define OPCODE(NAME, COUNT, CLASS) \
        case Opcode##NAME: \
            return new CLASS(*cast<CLASS>());
        OPCODE_LIST(OPCODE)
#undef OPCODE
    }
}

Context *Instruction::getContext() const {
    assert(getParent());
    return getParent()->getContext();
}

void CallInst::dump(std::ostream &os) {
    dumpName(os) << " = call ";
    callee->dumpAsOperand(os);
    os << "(" << dump_str(operands()) << ")";
}

Type *CallInst::getCalleeType() const {
    return callee->getType();
}
