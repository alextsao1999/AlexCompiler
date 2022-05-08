//
// Created by Alex on 2022/3/13.
//

#ifndef DRAGON_LOWERING_H
#define DRAGON_LOWERING_H

#include "Function.h"
#include "MachinePass.h"
///< Pattern DAG Builder
class Lowering : public MachinePass, public InstVisitor<Lowering, PatternNode *> {
public:
    std::map<Value *, PatternNode *> mapValueToNode;
    MachineBlock *block = nullptr;
    Function *curFunc = nullptr;
    void runOnFunction(Function &function) override {
        curFunc = &function;
        mapValueToNode.clear();

        auto *TI = function.getTargetInfo();
        assert(TI);
        int I = 0;
        for (auto &Param: function.getParams()) {
            auto *Node = TI->loweringArgument(function.dag, Param.get(), I++);
            mapValueToNode[Param.get()] = Node;
        }

        for (auto &BB: function) {
            auto &MBB = function.mapBlocks[&BB];
            MBB = new MachineBlock(BB.dumpOperandToString(), &BB);
            function.blocks.push_back(MBB);
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
        block = &mbb;
        auto *BB = mbb.getOrigin();
        std::vector<PatternNode *> RootNodes;
        for (auto &Inst: BB->instrs().reverse()) {
            if (mapValueToNode.find(&Inst) == mapValueToNode.end()) {
                auto *Node = InstVisitor::visit(&Inst);
                RootNodes.push_back(Node);
            }
        }

        auto *Node = PatternNode::createNode<RootNode>(RootNodes, &mbb);
        curFunc->dag.setRootNode(Node);
        mbb.setRootNode(Node);
    }

    template<typename T, typename ...Args>
    T *newNode(Value *val, Args &&...args) {
        auto *Node = new T(std::forward<Args>(args)...);
        Node->setType(val->getType());
        curFunc->dag.addNode(Node);
        mapValueToNode[val] = Node;
        return Node;
    }

    PatternNode *visit(Value *val) {
        if (mapValueToNode.find(val) != mapValueToNode.end()) {
            return mapValueToNode[val];
        }
        if (val->isBasicBlock()) {
            return newNode<BlockAddress>(val, val->cast<BasicBlock>());
        }
        if (val->isConstant()) {
            return newNode<ConstantNode>(val, val->cast<Constant>());
        }
        auto *Node = InstVisitor::visit(val);
        if (Node == nullptr) {
            std::cerr << "Don't know how to handle " << val->dumpOperandToString() << std::endl;
        }
        return Node;
    }

    PatternNode *visitRet(RetInst *value) override {
        // FIXME: better way to lower ret void
        if (value->getRetVal() == nullptr) {
            return newNode<ReturnNode>(value);
        }
        auto *TI = curFunc->getTargetInfo();
        auto *Node = TI->loweringReturn(curFunc->dag, visit(value->getRetVal()));
        return newNode<ReturnNode>(value, Node);
    }

    PatternNode *visitLoad(LoadInst *value) override {
        return newNode<LoadNode>(value, visit(value->getPtr()));
    }

    PatternNode *visitBinary(BinaryInst *value) override {
        auto BinOp = value->getOp();
        auto *Node = PatternNode::New(BinOp, 2);
        mapValueToNode[value] = Node;
        curFunc->dag.addNode(Node);
        auto *LHS = visit(value->getLHS());
        auto *RHS = visit(value->getRHS());
        Node->setChild(0, LHS);
        Node->setChild(1, RHS);
        Node->setType(value->getType());
        return Node;
    }

    PatternNode *visitGetPtr(GetPtrInst *value) override {
        return InstVisitor::visitGetPtr(value);
    }

    PatternNode *visitAlloca(AllocaInst *value) override {
        return InstVisitor::visitAlloca(value);
    }

    PatternNode *visitCopy(CopyInst *value) override {
        return InstVisitor::visitCopy(value);
    }

    PatternNode *visitAssign(AssignInst *value) override {
        return InstVisitor::visitAssign(value);
    }

    PatternNode *visitCondBr(CondBrInst *value) override {
        return newNode<CondJump>(value, visit(value->getCond()), visit(value->getTrueTarget()),
                                 visit(value->getFalseTarget()));
    }

    PatternNode *visitBr(BranchInst *value) override {
        return newNode<Jump>(value, visit(value->getTarget()));
    }

    PatternNode *visitPhi(PhiInst *value) override {
        return newNode<CopyFromReg>(value, value);
    }

    PatternNode *visitUnary(UnaryInst *value) override {
        return InstVisitor::visitUnary(value);
    }

    PatternNode *visitStore(StoreInst *value) override {
        return newNode<StoreNode>(value, visit(value->getPtr()), visit(value->getVal()));
    }

    PatternNode *visitCall(CallInst *value) override {
        return InstVisitor::visitCall(value);
    }

};

#endif //DRAGON_LOWERING_H
