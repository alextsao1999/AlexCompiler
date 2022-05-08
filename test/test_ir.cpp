//
// Created by Alex on 2022/3/13.
//

#include "test_common.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Dominance.h"
#include "SSAConstructor.h"
#include "Inliner.h"
#include "LoopAnalyse.h"
#include "LoopSimplify.h"
#include "GVN.h"
#include "BranchElim.h"
#include "ADCE.h"
#include "SCCP.h"

#include "PatternNode.h"

TEST(IR, BasicBlock) {
    Function F("test", Context.getVoidFunTy());

    auto *BB = BasicBlock::Create(&F, "BB");
    EXPECT_EQ(BB->getName(), "BB");

    IRBuilder Builder(BB);
    auto *Alloca = Builder.createAlloca(Context.getInt32Ty(), "V");
    BB->insertAfter(Alloca, Builder.createStore(Alloca, Context.getInt(22)));
    Builder.createRet(Builder.createLoad(Alloca));

    BB->dump(std::cout);
    EXPECT_EQ_VALUE(BB, R"(
        BB:			 --- preds=() succs=()
            %V = alloca i32, 1
            store i32* %V, i32 22
            %load = load i32* %V
            ret i32 %load
    )");

    auto *DomPass = new Dominance();
    auto *SSAPass = new SSAConstructor();

    DomPass->runOnFunction(F);
    SSAPass->runOnFunction(F);

    EXPECT_EQ_VALUE(BB, R"(
        BB:			 --- preds=() succs=()
            ret i32 22
    )");
}

TEST(IR, BBSplit) {
    Function F("test", Context.getVoidFunTy());
    auto *Entry = BasicBlock::Create(&F, "entry");
    IRBuilder Builder(Entry);
    auto *Alloca = Builder.createAlloca(Context.getInt32Ty(), "V");
    auto *Add = Builder.createAdd(Context.getInt(1), Context.getInt(2));
    Builder.createStore(Alloca, Add);
    Builder.createRet(Builder.createLoad(Alloca));

    Entry->split(Add, "NewBB");

    F.dump(std::cout);
}

TEST(IR, DeadBlock) {
    Function *F = new Function("test", Context.getVoidFunTy());
    auto *BB1 = BasicBlock::Create(F, "entry");
    auto *BB2 = BasicBlock::Create(F, "outer");
    auto *BB3 = BasicBlock::Create(F, "leave");

    IRBuilder Builder(BB1);
    Builder.createBr(BB3);

    Builder.setInsertPoint(BB2);
    Builder.createBr(BB3);

    Builder.setInsertPoint(BB3);
    Builder.createRet();

    auto *DomPass = new Dominance();
    auto *SSAPass = new SSAConstructor();

    DomPass->runOnFunction(*F);
    SSAPass->runOnFunction(*F);

    F->dump(std::cout);

    delete F;
}

TEST(IR, Module) {
    auto M = std::make_unique<Module>("test", Context);

    auto *F = M->createFunction("func1", Context.getVoidFunTy());
    auto *ParamX = F->addParam("x", Context.getInt32Ty());

    BasicBlock::Create(F, "entry");
    IRBuilder Builder(F);

    auto *A = Builder.createAlloca(Context.getInt32Ty(), "test");
    Builder.createStore(A, ParamX);

    auto *TrueBB = BasicBlock::Create(F, "if.true");
    auto *FalseBB = BasicBlock::Create(F, "if.false");
    auto *Leave = BasicBlock::Create(F, "leave");

    auto *Cmp = Builder.createNe(Builder.createLoad(A), Builder.getInt(66), "cmp");
    Builder.createCondBr(Cmp, TrueBB, FalseBB);

    Builder.setInsertPoint(TrueBB);
    Builder.createStore(A, Builder.getInt(33));
    Builder.createBr(Leave);

    Builder.setInsertPoint(FalseBB);
    Builder.createStore(A, Builder.getInt(44));
    Builder.createBr(Leave);

    Builder.setInsertPoint(Leave);
    auto *Load = Builder.createLoad(A);
    Builder.createRet(Load);

    PassManager PM;
    PM.addPass(new Dominance());
    PM.addPass(new SSAConstructor());
    PM.run(M.get());

    EXPECT_EQ_VALUE(F, R"(
        def func1(i32 %x) -> void {
            entry:			 --- preds=() succs=(%if.true, %if.false) doms=(%if.true, %if.false, %leave)
            %cmp = ne i32 %x, i32 66
            condbr i32 %cmp, %if.true, %if.false

            if.true:			 --- preds=(%entry) succs=(%leave) df=(%leave) idom=%entry
            br %leave

            if.false:			 --- preds=(%entry) succs=(%leave) df=(%leave) idom=%entry
            br %leave

            leave:			 --- preds=(%if.false, %if.true) succs=() idom=%entry
            %test = phi %if.true -> i32 33, %if.false -> i32 44
            ret i32 %test
        }
    )");

}

TEST(IR, IDF) {
    Function *F = new Function("test", Context.getVoidFunTy());
    BasicBlock *BB[] = {
            BasicBlock::Create(F, "entry"),
            BasicBlock::Create(F, "1"),
            BasicBlock::Create(F, "2"),
            BasicBlock::Create(F, "3"),
            BasicBlock::Create(F, "4"),
            BasicBlock::Create(F, "5"),
            BasicBlock::Create(F, "6"),
            BasicBlock::Create(F, "7"),
            BasicBlock::Create(F, "8"),
            BasicBlock::Create(F, "9"),
            BasicBlock::Create(F, "10"),
            BasicBlock::Create(F, "11"),
    };

    IRBuilder Builder(BB[0]);
    Builder.createBr(BB[1]);

    Builder.setInsertPoint(BB[1]);
    Builder.createBr(BB[2]);

    Builder.setInsertPoint(BB[2]);
    Builder.createCondBr(Builder.getInt(1), BB[3], BB[11]);

    Builder.setInsertPoint(BB[3]);
    Builder.createCondBr(Builder.getInt(1), BB[4], BB[8]);

    Builder.setInsertPoint(BB[4]);
    Builder.createBr(BB[5]);

    Builder.setInsertPoint(BB[5]);
    Builder.createBr(BB[6]);

    Builder.setInsertPoint(BB[6]);
    Builder.createCondBr(Builder.getInt(1), BB[5], BB[7]);

    Builder.setInsertPoint(BB[7]);
    Builder.createBr(BB[2]);

    Builder.setInsertPoint(BB[8]);
    Builder.createBr(BB[9]);

    Builder.setInsertPoint(BB[9]);
    Builder.createCondBr(Builder.getInt(1), BB[6], BB[10]);

    Builder.setInsertPoint(BB[10]);
    Builder.createBr(BB[8]);

    Builder.setInsertPoint(BB[11]);
    Builder.createRet();

    auto *DomPass = new Dominance();
    DomPass->runOnFunction(*F);

    EXPECT_EQ_VALUE(F, R"(def test() -> void {
        entry:    preds=() succs=(%1) doms=(%1)
        br %1

        1:    preds=(%entry) succs=(%2) doms=(%2) idom=%entry
        br %2

        2:    preds=(%7, %1) succs=(%3, %11) doms=(%3, %11) df=(%2) idom=%1
        condbr i32 1, %3, %11

        3:    preds=(%2) succs=(%4, %8) doms=(%4, %5, %6, %8) df=(%2) idom=%2
        condbr i32 1, %4, %8

        4:    preds=(%3) succs=(%5) df=(%5) idom=%3
        br %5

        5:    preds=(%6, %4) succs=(%6) df=(%6) idom=%3
        br %6

        6:    preds=(%9, %5) succs=(%5, %7) doms=(%7) df=(%2, %5) idom=%3
        condbr i32 1, %5, %7

        7:    preds=(%6) succs=(%2) df=(%2) idom=%6
        br %2

        8:    preds=(%10, %3) succs=(%9) doms=(%9) df=(%6, %8) idom=%3
        br %9

        9:    preds=(%8) succs=(%6, %10) doms=(%10) df=(%6, %8) idom=%8
        condbr i32 1, %6, %10

        10:    preds=(%9) succs=(%8) df=(%8) idom=%9
        br %8

        11:    preds=(%2) succs=() idom=%2
        ret
    })");

    F->getEntryBlock()->calculateLevel();

    IDFCalculator Calc;
    Calc.calulate({BB[1], BB[3], BB[4], BB[7]});
    EXPECT_EQ(dump_str(Calc.IDF), "%2, %5, %6");
    EXPECT_EQ(dump_str(Calc.calc({BB[1], BB[3], BB[4], BB[7]})), "%2, %5, %6");

    Calc.calulate({BB[6]});
    EXPECT_EQ(dump_str(Calc.IDF), "%2, %5, %6");
    EXPECT_EQ(dump_str(Calc.calc({BB[6]})), "%2, %5, %6");

    F->getSubList().clear();
    BasicBlock *BBNews[] = {
            BasicBlock::Create(F, "entry"),
            BasicBlock::Create(F, "if.true"),
            BasicBlock::Create(F, "if.false"),
            BasicBlock::Create(F, "leave")
    };
    Builder.setInsertPoint(BBNews[0]);
    Builder.createCondBr(Builder.getInt(0), BBNews[1], BBNews[2]);

    Builder.setInsertPoint(BBNews[1]);
    Builder.createBr(BBNews[3]);

    Builder.setInsertPoint(BBNews[2]);
    Builder.createBr(BBNews[3]);

    Builder.setInsertPoint(BBNews[3]);
    Builder.createRet();

    DomPass->runOnFunction(*F);

    EXPECT_EQ_VALUE(F, R"(def test() -> void {
        entry:    preds=() succs=(%if.true, %if.false) doms=(%if.true, %if.false, %leave)
        condbr i32 0, %if.true, %if.false

        if.true:    preds=(%entry) succs=(%leave) df=(%leave) idom=%entry
        br %leave

        if.false:    preds=(%entry) succs=(%leave) df=(%leave) idom=%entry
        br %leave

        leave:    preds=(%if.false, %if.true) succs=() idom=%entry
        ret
})");

    F->getEntryBlock()->calculateLevel();
    Calc.calulate({BBNews[1], BBNews[2]});
    EXPECT_EQ(dump_str(Calc.IDF), "%leave");
    EXPECT_EQ(dump_str(Calc.calc({BBNews[1], BBNews[2]})), "%leave");

    delete F;
}

Function *createFunc1() {
    auto *F = new Function("func1", Context.getFunctionTy(Context.getInt32Ty(), {Context.getInt32Ty()}));
    auto *ParamX = F->addParam("x", Context.getInt32Ty());

    BasicBlock::Create(F, "entry");
    IRBuilder Builder(F);

    auto *A = Builder.createAlloca(Context.getInt32Ty(), "test");
    Builder.createStore(A, ParamX);

    auto *TrueBB = BasicBlock::Create(F, "if.true");
    auto *FalseBB = BasicBlock::Create(F, "if.false");
    auto *Leave = BasicBlock::Create(F, "leave");

    auto *Add = Builder.createAdd(Builder.createLoad(A), Builder.getInt(1));
    auto *Cmp = Builder.createNe(Builder.getInt(77), Builder.getInt(66), "cmp");
    Builder.createCondBr(Cmp, TrueBB, FalseBB);

    Builder.setInsertPoint(TrueBB);
    Builder.createStore(A, ParamX);
    Builder.createBr(Leave);

    Builder.setInsertPoint(FalseBB);
    Builder.createStore(A, Builder.createAdd(Builder.createLoad(A), Builder.getInt(1)));
    Builder.createBr(Leave);

    Builder.setInsertPoint(Leave);
    auto *Load = Builder.createLoad(A);
    Builder.createRet(Load);

    return F;
}

TEST(IR, SSA) {
    auto M = std::make_unique<Module>("test", Context);

    auto *Func1 = createFunc1();
    M->append(Func1);

    auto *Main = Function::Create(M.get(), "main", Context.getFunctionTy(Context.getInt32Ty(), {}));
    BasicBlock *Entry = BasicBlock::Create(Main, "entry");
    IRBuilder Builder(Main);
    auto *Alloca = Builder.createAlloca(Context.getInt32Ty(), "value");
    auto *Ret = Builder.createCall(Func1, {Builder.getInt(666)}, "call");
    Builder.createStore(Alloca, Ret);
    Builder.createRet(Builder.createLoad(Alloca));

    PassManager PM;
    // add passes
    PM.addPass(new Inliner());
    PM.addPass(new Dominance());
    PM.addPass(new SSAConstructor());
    PM.addPass(new GVN());
    PM.addPass(new BranchElim());
    PM.addPass(new Dominance());
    PM.addPass(new ADCE());
    //PM.addPass(new SSADestructor());

    // run passes
    PM.run(M.get());

    M->dump(std::cout);

}

TEST(IR, Loop) {
    Function *F = new Function("test", Context.getVoidFunTy());

    BasicBlock *BB[] = {
            BasicBlock::Create(F, "entry"),
            BasicBlock::Create(F, "loop.body.1"),
            BasicBlock::Create(F, "loop.cond.1"),
            BasicBlock::Create(F, "loop.cond.1.1"),
            BasicBlock::Create(F, "loop.body.1.1"),
            BasicBlock::Create(F, "loop.cond.2"),
            BasicBlock::Create(F, "loop.body.2"),
            BasicBlock::Create(F, "leave"),
    };

    IRBuilder Builder;
    Builder.setInsertPoint(BB[0]);
    Builder.createBr(BB[1]);

    Builder.setInsertPoint(BB[1]);
    Builder.createBr(BB[3]);

    Builder.setInsertPoint(BB[2]);
    Builder.createCondBr(Builder.getInt(1), BB[1], BB[5]);

    Builder.setInsertPoint(BB[3]);
    Builder.createCondBr(Builder.getInt(1), BB[4], BB[2]);

    Builder.setInsertPoint(BB[4]);
    Builder.createBr(BB[3]);

    Builder.setInsertPoint(BB[5]);
    Builder.createCondBr(Builder.getInt(1), BB[6], BB[7]);

    Builder.setInsertPoint(BB[6]);
    Builder.createBr(BB[5]);

    Builder.setInsertPoint(BB[7]);
    Builder.createRet();

    auto *DomPass = new Dominance();
    auto *SSAPass = new SSAConstructor();
    auto *LoopPass = new LoopAnalyse();

    DomPass->runOnFunction(*F);
    SSAPass->runOnFunction(*F);
    LoopPass->runOnFunction(*F);

    for (auto &Loop: F->loops) {
        std::cout << "Loop: " << Loop.getHeader()->getName() << std::endl;
    }

    delete F;
}

TEST(IR, LoopSimplify) {
    Function F("test", Context.getVoidFunTy());

    BasicBlock *BB[] = {
            BasicBlock::Create(&F, "entry"),
            BasicBlock::Create(&F, "loop.body"),
            BasicBlock::Create(&F, "loop.cond"),
            BasicBlock::Create(&F, "leave"),
    };

    IRBuilder Builder;
    Builder.setInsertPoint(BB[0]);
    Builder.createBr(BB[1]);

    Builder.setInsertPoint(BB[1]);
    Builder.createBr(BB[2]);

    Builder.setInsertPoint(BB[2]);
    Builder.createCondBr(Builder.getInt(1), BB[1], BB[3]);

    Builder.setInsertPoint(BB[3]);
    Builder.createRet();

    auto *DomPass = new Dominance();
    auto *SSAPass = new SSAConstructor();
    auto *LoopPass = new LoopAnalyse();
    auto *LoopSimplifyPass = new LoopSimplify();

    DomPass->runOnFunction(F);
    SSAPass->runOnFunction(F);
    LoopPass->runOnFunction(F);
    LoopSimplifyPass->runOnFunction(F);

    F.dump(std::cout);
}

TEST(PN, 0) {
    auto *add = PatternNode::New(Pattern::Add, 3);
    std::cout << add->getNumOperands();

    auto *Node = add->as<SubNode>();

    auto *test = PatternNode::createNode<CallNode>({add, Node, add});
    std::cout << test->getNumOperands();


}