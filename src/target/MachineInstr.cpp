//
// Created by Alex on 2022/3/24.
//

#include "MachineInstr.h"
#include "MachineBlock.h"
#include "Function.h"

void MachineInstr::dump(std::ostream &os) {
    switch ((TargetOpcode) opcode) {
        case TargetAdd:
            os << "add";
            break;
        case TargetSub:
            os << "sub";
            break;
        case TargetMul:
            os << "mul";
            break;
        case TargetDiv:
            os << "div";
            break;
        case TargetMod:
            os << "mod";
            break;
        case TargetAnd:
            os << "and";
            break;
        case TargetOr:
            os << "or";
            break;
        case TargetXor:
            os << "xor";
            break;
        case TargetShl:
            os << "shl";
            break;
        case TargetShr:
            os << "shr";
            break;
        case TargetEq:
            os << "eq";
            break;
        case TargetNe:
            os << "ne";
            break;
        case TargetLt:
            os << "lt";
            break;
        case TargetGt:
            os << "gt";
            break;
        case TargetLe:
            os << "le";
            break;
        case TargetGe:
            os << "ge";
            break;
        case TargetNeg:
            os << "neg";
            break;
        case TargetNot:
            os << "not";
            break;
        case TargetMove:
            os << "move";
            break;
        case TargetCBr:
            os << "cbr";
            break;
        case TargetBr:
            os << "br";
            break;
        case TargetCall:
            os << "call";
            break;
        case TargetCmp:
            os << "cmp";
            break;
        case TargetRet:
            os << "ret";
            break;
        case TargetNone:
            os << "none";
            break;
        default:
            os << "unknown";
            break;
    }
    os << " ";
    if (hasDef()) {
        dumpOp(os << "def:", defOp);
    }
    for (auto &op : ops()) {
        if (hasDef() || &op != op_begin().getPointer()) {
            os << ", ";
        }
        dumpOp(os, op);
    }
}

void MachineInstr::dumpOp(std::ostream &os, Operand &op) {
    auto *F = getParent()->getFunction();
    assert(F);
    auto *TI = F->getTargetInfo();
    switch (op.kind) {
        case Operand::Nop:
            os << "none";
            break;
        case Operand::Reg:
            if (op.regOp.isVirReg()) {
                os << "(vreg #" << op.regOp.getRegNo() << ")";
            } else {
                //os << "(reg #" << TI->getRegName(op.regOp.getRegNo()) << ")";
                os << TI->getRegName(op.regOp.getRegNo());
            }
            break;
        case Operand::Imm:
            os << op.value.imm;
            break;
        case Operand::Label:
            os << "(label " << op.value.addr->name << ")";
            break;
        case Operand::Slot:
            os << "(slot " << op.value.slot << ")";
            break;
    }
}
