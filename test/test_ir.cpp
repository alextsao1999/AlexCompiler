//
// Created by Alex on 2022/3/13.
//

#include "lest.hpp"
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
    EXPECT(SplitAndTrim(V->dumpToString()) == SplitAndTrim(EXPECTED))

const lest::test Specification[] = {
        CASE("BasicBlock Test") {
            Function *F = new Function("test", Context.getVoidFunTy());

            auto *BB = BasicBlock::Create(F, "BB");
            EXPECT(BB->getName() == "BB");

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
                BB:
                    ret i32 22
            )");

            delete F;
        },

        CASE("Module Test") {
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
        },


};

int main(int argc, char * argv[]) {
    return lest::run(Specification, argc, argv);
}
