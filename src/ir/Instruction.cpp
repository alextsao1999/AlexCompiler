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
    for (auto &Op: operands()) {
        Op->dumpAsOperand(os);
        os << " ";
    }

}

const std::string &Instruction::getName() {
    auto &ST = getParent()->getParent()->getSymbolTable();
    return ST.getName(this);
}

void Instruction::setName(std::string_view name) {
    auto &ST = getParent()->getParent()->getSymbolTable();
    ST.setName(this, name);
}

Instruction::Instruction(Opcode opcode, std::initializer_list<Value *> values) : opcode(opcode), trailingOperands(new Use[values.size()]) {
    numOperands = values.size();
    auto *Operand = getTrailingOperand();
    for (auto &Value: values) {
        Operand->setUser(this);
        Operand->set(Value);
        Operand++;
    }
}


BranchInst::BranchInst(BasicBlock *target) : TerminatorInst(OpcodeBr, {target}) {

}

CondBrInst::CondBrInst(Value *cond, BasicBlock *trueTarget, BasicBlock *falseTarget) : TerminatorInst(OpcodeCondBr,
                                                                                                      {cond, trueTarget,
                                                                                                       falseTarget}) {}

