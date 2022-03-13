//
// Created by Alex on 2022/3/8.
//

#include "Instruction.h"
#include "BasicBlock.h"
#include "Function.h"

void Instruction::dump(std::ostream &os, int level) {
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
        switch (opcode) {
            default: unreachable();
#define OPCODE(NAME, COUNT) \
        case Opcode##NAME: \
            os << #NAME; \
            break;
            OPCODE_LIST(OPCODE)
#undef OPCODE
        }
    }

    os << " ";

    DUMP_REF(os, operands(), V, {
        V->dumpAsOperand(os);
    });

}

const std::string &Instruction::getName() {
    auto &ST = getParent()->getParent()->getSymbolTable();
    return ST.getName(this);
}

void Instruction::setName(std::string_view name) {
    auto &ST = getParent()->getParent()->getSymbolTable();
    ST.setName(this, name);
}

Instruction::Instruction(BasicBlock *parent, Opcode opcode) : opcode(opcode), numOperands(OpcodeNum[opcode]) {
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

PhiInst *PhiInst::Create(BasicBlock *bb) {
    auto *Phi = new PhiInst();
    bb->addPhi(Phi);
    return Phi;
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
}

BasicBlock *PhiInst::getIncomingBlock(Use &use) {
    auto Index = &use - getTrailingOperand();
    assert(Index >= 0 && Index < numOperands);
    return incomingBlocks[Index]->cast<BasicBlock>();
}

BasicBlock *PhiInst::getIncomingBlock(size_t i) {
    assert(i < numOperands);
    return incomingBlocks[i]->cast<BasicBlock>();
}

void PhiInst::dump(std::ostream &os, int level) {
    os << "%" << getName() << " = phi ";
    DUMP_REF(os, operands(), V, {
        getIncomingBlock(V)->dumpAsOperand(os);
        os << " ";
        V->dumpAsOperand(os);
    });

}
