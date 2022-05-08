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
            auto PhyReg = Register::phy(I++);

            //auto *Node = TI->loweringArgument(function.dag, Param.get(), I++);
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

    Operand getValueOp(Value *V) {
        if (V->isConstant()) {
            if (auto *Int = V->as<IntConstant>()) {
                return Operand::imm(Int->getVal());
            }
        } else {
            return Operand::reg(getValueReg(V));
        }
        return Operand::undefined();
    }

    MachineInstr *visitCondBr(CondBrInst *value, MachineBlock &mbb) override {
        return MIBuilder()
                .setOpcode(TargetCBr)
                .addOp(getValueOp(value->getCond()))
                .build();
    }

    MachineInstr *visitBr(BranchInst *value, MachineBlock &mbb) override {
        return MIBuilder()
                .setOpcode(TargetBr)
                .build();
    }

    MachineInstr *visitCopy(CopyInst *value, MachineBlock &mbb) override {
        return MIBuilder().setOpcode(TargetMove)
                .addDef(getValueReg(value))
                .addOp(getValueOp(value->getVal()))
                .build();
    }

    MachineInstr *visitAssign(AssignInst *value, MachineBlock &mbb) override {
        return MIBuilder()
                .setOpcode(TargetMove)
                .addDef(getValueReg(value->getLHS()))
                .addOp(getValueOp(value->getRHS()))
                .build();
    }

    MachineInstr *visitBinary(BinaryInst *value, MachineBlock &mbb) override {
        return MIBuilder().setOpcode(value->getOp())
                .addDef(getValueReg(value))
                .addOp(getValueOp(value->getLHS()))
                .addOp(getValueOp(value->getRHS()))
                .build();
    }

    MachineInstr *visitRet(RetInst *value, MachineBlock &mbb) override {
        if (auto *RetVal = value->getRetVal())
            return MIBuilder().setOpcode(TargetRet)
                    .addOp(getValueOp(value->getRetVal()))
                    .build();
        return MIBuilder().setOpcode(TargetRet).build();
    }

    MachineInstr *visitCall(CallInst *value, MachineBlock &mbb) override {
        MIBuilder builder;
        builder.setOpcode(TargetCall);
        builder.addDef(getValueReg(value));
        for (int i = 0; i < value->getOperandNum(); ++i) {

        }
        return builder.build();
    }

    MachineInstr *visitLoad(LoadInst *value, MachineBlock &mbb) override {
        return InstVisitor::visitLoad(value, mbb);
    }

    MachineInstr *visitAlloca(AllocaInst *value, MachineBlock &mbb) override {
        return InstVisitor::visitAlloca(value, mbb);
    }
};


#endif //DRAGON_RISCVLOWERING_H
