//
// Created by Alex on 2022/3/13.
//

#include "lest.hpp"
#include "Function.h"
#include "IRBuilder.h"
#include "Dominance.h"
#include "SSABuilder.h"

Context Context;

const lest::test Specification[] = {
        CASE("BasicBlock Test") {
            Function *F = new Function("test", Context.getVoidFunTy());

            auto *BB = BasicBlock::Create(F, "BB");
            EXPECT(BB->getName() == "BB");

            IRBuilder Builder(BB);
            auto *Alloca = Builder.createAlloca(Context.getInt32Ty(), "V");
            BB->insertAfter(Alloca, Builder.createStore(Alloca, Context.getInt(22)));
            Builder.createRet(Builder.createLoad(Alloca));

            EXPECT(BB->dumpToString() == "BB: preds=()  succs=()  doms=()  df=()\n"
                                         "%V = alloca i32, 1\n"
                                         "store i32* %V, i32 22\n"
                                         "%load = load i32* %V\n"
                                         "ret i32 %load");

            auto *DomPass = new Dominance();
            auto *SSAPass = new SSABuilder();

            DomPass->runOnFunction(F);
            SSAPass->runOnFunction(F);

            EXPECT(BB->dumpToString() == "BB: preds=()  succs=()  doms=()  df=()\n"
                                         "ret i32 22");

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
            PM.addPass(new SSABuilder());
            PM.run(M.get());

            F->dump(std::cout);
        },

};

int main(int argc, char * argv[]) {
    return lest::run(Specification, argc, argv);
}
