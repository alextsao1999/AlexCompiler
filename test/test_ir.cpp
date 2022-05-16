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

    CHECK_OR_DUMP(BB, R"(
BB.0:    preds=() succs=()
%V.0 = alloca i32
store i32* %V.0, i32 22
%load.0 = load i32* %V.0
ret i32 %load.0
)");

    auto *DomPass = new Dominance();
    auto *SSAPass = new SSAConstructor();

    DomPass->runOnFunction(F);
    SSAPass->runOnFunction(F);

    CHECK_OR_DUMP(BB, R"(
BB.0:    preds=() succs=()
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

    CHECK_OR_DUMP(Entry, R"(
entry.0:    preds=(%NewBB.0) succs=()
%add.0 = add i32 1, i32 2
store i32* %V.0, i32 %add.0
%load.0 = load i32* %V.0
ret i32 %load.0
)");

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

    CHECK_OR_DUMP(F, R"(
def test() -> void {
entry.0:    preds=() succs=(%leave.0) doms=(%leave.0)
br %leave.0

outer.0:    preds=() succs=(%leave.0) df=(%leave.0)
br %leave.0

leave.0:    preds=(%outer.0, %entry.0) succs=() idom=%entry.0
ret
}
)");

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

    CHECK_OR_DUMP(F, R"(
def func1(i32 %x) -> void {
entry.0:    preds=() succs=(%if.true.0, %if.false.0) doms=(%if.true.0, %if.false.0, %leave.0)
%cmp.0 = ne i32 %x, i32 66
condbr i32 %cmp.0, %if.true.0, %if.false.0

if.true.0:    preds=(%entry.0) succs=(%leave.0) df=(%leave.0) idom=%entry.0
br %leave.0

if.false.0:    preds=(%entry.0) succs=(%leave.0) df=(%leave.0) idom=%entry.0
br %leave.0

leave.0:    preds=(%if.false.0, %if.true.0) succs=() idom=%entry.0
%test.0 = phi [%if.true.0: i32 33], [%if.false.0: i32 44]
ret i32 %test.0
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

    CHECK_OR_DUMP(F, R"(
def test() -> void {
entry.0:    preds=() succs=(%1.0) doms=(%1.0)
br %1.0

1.0:    preds=(%entry.0) succs=(%2.0) doms=(%2.0) idom=%entry.0
br %2.0

2.0:    preds=(%7.0, %1.0) succs=(%3.0, %11.0) doms=(%3.0, %11.0) df=(%2.0) idom=%1.0
condbr i32 1, %3.0, %11.0

3.0:    preds=(%2.0) succs=(%4.0, %8.0) doms=(%4.0, %5.0, %6.0, %8.0) df=(%2.0) idom=%2.0
condbr i32 1, %4.0, %8.0

4.0:    preds=(%3.0) succs=(%5.0) df=(%5.0) idom=%3.0
br %5.0

5.0:    preds=(%6.0, %4.0) succs=(%6.0) df=(%6.0) idom=%3.0
br %6.0

6.0:    preds=(%9.0, %5.0) succs=(%5.0, %7.0) doms=(%7.0) df=(%2.0, %5.0) idom=%3.0
condbr i32 1, %5.0, %7.0

7.0:    preds=(%6.0) succs=(%2.0) df=(%2.0) idom=%6.0
br %2.0

8.0:    preds=(%10.0, %3.0) succs=(%9.0) doms=(%9.0) df=(%6.0, %8.0) idom=%3.0
br %9.0

9.0:    preds=(%8.0) succs=(%6.0, %10.0) doms=(%10.0) df=(%6.0, %8.0) idom=%8.0
condbr i32 1, %6.0, %10.0

10.0:    preds=(%9.0) succs=(%8.0) df=(%8.0) idom=%9.0
br %8.0

11.0:    preds=(%2.0) succs=() idom=%2.0
ret
}

)");

    //F->getEntryBlock()->calculateLevel();
    //IDFCalculator Calc;
    //EXPECT_EQ(dump_str(Calc.calc({BB[1], BB[3], BB[4], BB[7]})), "%2, %5, %6");
    //EXPECT_EQ(dump_str(Calc.calc({BB[6]})), "%2, %5, %6");

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

    CHECK_OR_DUMP(F, R"(
def test() -> void {
entry.1:    preds=() succs=(%if.true.0, %if.false.0) doms=(%leave.0, %if.false.0, %if.true.0)
condbr i32 0, %if.true.0, %if.false.0

if.true.0:    preds=(%entry.1) succs=(%leave.0) df=(%leave.0) idom=%entry.1
br %leave.0

if.false.0:    preds=(%entry.1) succs=(%leave.0) df=(%leave.0) idom=%entry.1
br %leave.0

leave.0:    preds=(%if.false.0, %if.true.0) succs=() idom=%entry.1
ret
}
)");

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
    CHECK_OR_DUMP(Main, "def main() -> i32 {\n"
                        "entry.0:    preds=() succs=() \n"
                        "%value.0 = alloca i32\n"
                        "%call.0 = call i32 @func1(i32 666)\n"
                        "store i32* %value.0, i32 %call.0\n"
                        "%load.0 = load i32* %value.0\n"
                        "ret i32 %load.0\n"
                        "}");

    // run passes
    PM.run(M.get());

    //M->dump(std::cout);
    CHECK_OR_DUMP(Main, "def main() -> i32 {\n"
                        "entry.split.0:    preds=() succs=(%entry.inlined.0) doms=(%entry.inlined.0) \n"
                        "br %entry.inlined.0\n"
                        "\n"
                        "entry.inlined.0:    preds=(%entry.split.0) succs=(%if.true.inlined.0, %if.false.inlined.0) doms=(%if.true.inlined.0, %if.false.inlined.0, %leave.inlined.0) idom=%entry.split.0\n"
                        "condbr i32 1, %if.true.inlined.0, %if.false.inlined.0\n"
                        "\n"
                        "if.true.inlined.0:    preds=(%entry.inlined.0) succs=(%leave.inlined.0) df=(%leave.inlined.0) idom=%entry.inlined.0\n"
                        "br %leave.inlined.0\n"
                        "\n"
                        "if.false.inlined.0:    preds=(%entry.inlined.0) succs=(%leave.inlined.0) df=(%leave.inlined.0) idom=%entry.inlined.0\n"
                        "br %leave.inlined.0\n"
                        "\n"
                        "leave.inlined.0:    preds=(%if.false.inlined.0, %if.true.inlined.0) succs=(%entry.0) doms=(%entry.0) idom=%entry.inlined.0\n"
                        "%0 = phi [%if.true.inlined.0: i32 666], [%if.false.inlined.0: i32 667]\n"
                        "br %entry.0\n"
                        "\n"
                        "entry.0:    preds=(%leave.inlined.0) succs=() idom=%leave.inlined.0\n"
                        "ret i32 %0\n"
                        "}");

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
