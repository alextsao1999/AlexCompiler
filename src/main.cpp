//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "PassManager.h"
#include "Dominance.h"
#include "SSABuilder.h"

int main(int argc, char **argv) {
    Context Context;
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

    return 0;
}