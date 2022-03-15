//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "PassManager.h"
#include "Dominance.h"
#include "SSAConstructor.h"
#include "Inliner.h"
#include "GVN.h"
#include "BranchElim.h"
#include "ADCE.h"
#include "SSADestructor.h"

static Context Context;

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

int main(int argc, char **argv) {
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
    PM.addPass(new ADCE());
    PM.addPass(new SSADestructor());

    // run passes
    PM.run(M.get());

    M->dump(std::cout);

    return 0;
}