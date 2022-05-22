//
// Created by Alex on 2022/5/8.
//

#ifndef DRAGON_RISCVLOWERING_H
#define DRAGON_RISCVLOWERING_H

#include <MachinePass.h>
#include <RISCVTarget.h>
class RISCVLowering : public MachinePass, public InstVisitor<RISCVLowering, MachineInstr *, MachineBlock &> {
public:
    Function *curFunc = nullptr;
    std::map<Value *, RegID> mapValueToReg;
    int allocateVirReg = 0;
    void runOnFunction(Function &function) override {
        curFunc = &function;

        auto *TI = function.getTargetInfo();
        assert(TI);
        int I = 0;
        for (auto &Param: function.getParams()) {
            auto PhyReg = Register::phy(RISCV::A0 + I++);
            mapValueToReg[Param.get()] = PhyReg;
        }

        for (auto &BB: function) {
            auto &MBB = function.mapBlocks[&BB];
            if (!MBB) {
                MBB = new MachineBlock(BB.dumpOperandToString(), &BB);
                function.blocks.push_back(MBB);
            }
            buildOnBlock(*MBB);
        }

        ///< Update successor and predecessor
        for (auto &MBB: function.blocks) {
            for (auto *Succ: MBB.getOrigin()->succs()) {
                assert(function.mapBlocks[Succ]);
                MBB.succs.push_back(function.mapBlocks[Succ]);
            }
            for (auto *Pred: MBB.getOrigin()->preds()) {
                assert(function.mapBlocks[Pred]);
                MBB.preds.push_back(function.mapBlocks[Pred]);
            }
        }

    }

    void buildOnBlock(MachineBlock &mbb) {
        auto *Block = mbb.getOrigin();
        for (auto &I: *Block) {
            if(auto *MI = visit(&I, mbb)){
                mbb.append(MI);
            }
        }
    }

    Register getValueReg(Value *V) {
        if (mapValueToReg.count(V)) {
            return mapValueToReg[V];
        }
        mapValueToReg[V] = Register::vir(allocateVirReg++);
        return mapValueToReg[V];
    }

    Operand getBlockLabel(BasicBlock *bb) {
        auto *F = bb->getParent();
        assert(F && F->mapBlocks[bb]);
        return Operand::label(F->mapBlocks[bb]);
    }

    Operand getImmOp(int64_t imm) {
        return Operand::imm(imm);
    }

    Operand getValueOp(Value *V) {
        if (V->isConstant()) {
            if (auto *Int = V->as<IntConstant>()) {
                if (Int->getVal() == 0) {
                    return Operand::reg(RISCV::Zero);
                }
                return Operand::imm(Int->getVal());
            }
        } else {
            return Operand::reg(getValueReg(V));
        }
        return Operand::undefined();
    }

    MachineInstr *visitCondBr(CondBrInst *value, MachineBlock &mbb) override {
        /*if (auto *Bin = value->getCond()->as<BinaryInst>()) {
            if (Bin->isOnlyUsedOnce()) {
                if (Bin->getOp() == BinaryOp::Lt) {
                    mbb.append(MIBuilder()
                                       .setOpcode(RISCV::BLT)
                                       .addOp(getValueOp(Bin->getLHS()))
                                       .addOp(getValueOp(Bin->getRHS()))
                                       .addOp(getBlockLabel(value->getTrueTarget()))
                                       .build());
                    return nullptr;
                }
            }
        }*/
        mbb.append(MIBuilder()
                           .setOpcode(RISCV::BNE)
                           .addOp(getValueOp(value->getCond()))
                           .addOp(Operand::reg(RISCV::Zero))
                           .addOp(getBlockLabel(value->getTrueTarget()))
                           .build());

        return MIBuilder()
                .setOpcode(TargetBr)
                .addOp(getBlockLabel(value->getFalseTarget()))
                .build();
    }

    MachineInstr *buildMove(Register LHS, Operand RHS) {
        return MIBuilder()
                .setOpcode(RISCV::MV)
                .addDef(LHS)
                .addOp(RHS)
                .build();
        /*MIBuilder AB;
        AB.setOpcode(RISCV::ADDI)
                .addDef(LHS)
                .addOp(RHS)
                .addUse(Register::phy(RISCV::Zero));
        return AB.build();*/
    }

    MachineInstr *visitCall(CallInst *value, MachineBlock &mbb) override {
        MIBuilder builder;
        builder.setOpcode(TargetCall);
        builder.addDef(Register::phy(RISCV::A0));
        for (int i = 0; i < value->getArgNum(); ++i) {
            mbb.append(buildMove(Register::phy(RISCV::A0 + i), getValueOp(value->getArg(i))));
        }
        builder.build();
        auto *Callee = value->getCallee();
        assert(Callee);
        auto *Entry = Callee->getEntryBlock();
        mbb.append(MIBuilder().setOpcode(RISCV::CALL).addOp(getBlockLabel(Entry)).build());
        auto Reg = getValueReg(value); // get a virtual register for the return value
        return buildMove(Reg, Operand::reg(Register::phy(RISCV::A0)));
    }

    MachineInstr *visitBr(BranchInst *value, MachineBlock &mbb) override {
        return MIBuilder()
                .setOpcode(TargetBr)
                .addOp(getBlockLabel(value->getTarget()))
                .build();
    }

    MachineInstr *visitCopy(CopyInst *value, MachineBlock &mbb) override {
        return buildMove(getValueReg(value), getValueOp(value->getVal()));
    }

    MachineInstr *visitAssign(AssignInst *value, MachineBlock &mbb) override {
        return buildMove(getValueReg(value), getValueOp(value->getRHS()));
    }

    MachineInstr *visitBinary(BinaryInst *value, MachineBlock &mbb) override {
        return MIBuilder().setOpcode(value->getOp())
                .addDef(getValueReg(value))
                .addOp(getValueOp(value->getLHS()))
                .addOp(getValueOp(value->getRHS()))
                .build();
    }

    MachineInstr *visitRet(RetInst *value, MachineBlock &mbb) override {
        if (auto *RetVal = value->getRetVal()) {
            mbb.append(buildMove(Register::phy(RISCV::A0), getValueOp(RetVal)));
        }
        return MIBuilder().setOpcode(TargetRet).build();
    }

    MachineInstr *visitLoad(LoadInst *value, MachineBlock &mbb) override {
        return InstVisitor::visitLoad(value, mbb);
    }

    MachineInstr *visitAlloca(AllocaInst *value, MachineBlock &mbb) override {
        return InstVisitor::visitAlloca(value, mbb);
    }
};


#endif //DRAGON_RISCVLOWERING_H
