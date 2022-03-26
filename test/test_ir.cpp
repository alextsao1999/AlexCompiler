//
// Created by Alex on 2022/3/13.
//

#include "gtest/gtest.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Dominance.h"
#include "SSAConstructor.h"

Context Context;

auto SplitAndTrim(const std::string &str) -> std::string {
    std::vector<std::string> Res;
    std::stringstream SS(str);
    std::string Item;
    while (std::getline(SS, Item)) {
        Res.push_back(Item);
    }
    static const char *TrimChars = "  \t\r\n";
    // Trim Each Line
    for (auto &Line: Res) {
        Line.erase(Line.find_last_not_of(TrimChars) + 1);
        Line.erase(0, Line.find_first_not_of(TrimChars));
    }
    // Remove Empty Line
    Res.erase(std::remove_if(Res.begin(), Res.end(), [](const std::string &str) {
        return str.empty();
    }), Res.end());
    // Join the Res
    std::stringstream SSRes;
    for (auto &Line: Res) {
        SSRes << Line << "\n";
    }
    return SSRes.str();
};

#define EXPECT_EQ_VALUE(V, EXPECTED) \
    EXPECT_EQ(SplitAndTrim(V->dumpToString()), SplitAndTrim(EXPECTED))

TEST(IR, BasicBlock) {
    Function *F = new Function("test", Context.getVoidFunTy());

    auto *BB = BasicBlock::Create(F, "BB");
    EXPECT_EQ(BB->getName(), "BB");

    IRBuilder Builder(BB);
    auto *Alloca = Builder.createAlloca(Context.getInt32Ty(), "V");
    BB->insertAfter(Alloca, Builder.createStore(Alloca, Context.getInt(22)));
    Builder.createRet(Builder.createLoad(Alloca));

    EXPECT_EQ_VALUE(BB, R"(
        BB:
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
        BB: doms=()  df=()
            ret i32 22
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

    EXPECT_EQ_VALUE(F, R"(
            def func1(i32 %x) -> void {
                entry:
                %cmp = ne i32 %x, i32 66
                condbr i32 %cmp, %if.true, %if.false

                if.true:
                br %leave

                if.false:
                br %leave

                leave:
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
    DomPass->runOnFunction(F);

    EXPECT_EQ_VALUE(F, R"(def test() -> void {
        entry: preds=() succs=(%1) doms=(%1) df=()
        br %1

        1: preds=(%entry) succs=(%2) doms=(%2) df=() idom=%entry
        br %2

        2: preds=(%7, %1) succs=(%3, %11) doms=(%3, %11) df=(%2) idom=%1
        condbr i32 1, %3, %11

        3: preds=(%2) succs=(%4, %8) doms=(%4, %5, %6, %8) df=(%2) idom=%2
        condbr i32 1, %4, %8

        4: preds=(%3) succs=(%5) doms=() df=(%5) idom=%3
        br %5

        5: preds=(%6, %4) succs=(%6) doms=() df=(%6) idom=%3
        br %6

        6: preds=(%9, %5) succs=(%5, %7) doms=(%7) df=(%2, %5) idom=%3
        condbr i32 1, %5, %7

        7: preds=(%6) succs=(%2) doms=() df=(%2) idom=%6
        br %2

        8: preds=(%10, %3) succs=(%9) doms=(%9) df=(%6, %8) idom=%3
        br %9

        9: preds=(%8) succs=(%6, %10) doms=(%10) df=(%6, %8) idom=%8
        condbr i32 1, %6, %10

        10: preds=(%9) succs=(%8) doms=() df=(%8) idom=%9
        br %8

        11: preds=(%2) succs=() doms=() df=() idom=%2
        ret
    })");

    BB[0]->calculateLevel();
    IDFCalculator Calc;
    Calc.calulate({BB[1], BB[3], BB[4], BB[7]});

    EXPECT_EQ(dump_str(Calc.IDF), "%2, %5, %6");

    delete F;
}

