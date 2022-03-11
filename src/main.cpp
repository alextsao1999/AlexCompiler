//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "PassManager.h"
#include "Dominance.h"
int main(int argc, char **argv) {
    Context Context;
    auto M = std::make_unique<Module>("test", Context);

    auto *F = M->createFunction("func1", Context.getVoidFunTy());

    BasicBlock::Create(F, "Entry");
    IRBuilder Builder(F);

    auto *A = Builder.createAlloca(Context.getInt32Ty(), "test");

    auto *Add = Builder.createAdd(Builder.getInt(1), A);
    auto *Mul = Builder.createMul(Add, Builder.getInt(6));
    auto *Load = Builder.createLoad(Mul, "val");


    auto *TrueBB = BasicBlock::Create(F, "br.true");
    auto *FalseBB = BasicBlock::Create(F, "br.false");
    auto *Leave = BasicBlock::Create(F, "leave");
    Builder.createCondBr(Load, TrueBB, FalseBB);

    Builder.setInsertPoint(TrueBB);
    Builder.createBr(Leave);

    Builder.setInsertPoint(FalseBB);
    Builder.createBr(Leave);

    Builder.setInsertPoint(Leave);
    Builder.createRet();

    PassManager PM;
    PM.addPass(new Dominance());
    PM.run(M.get());

    F->dump(std::cout);

    return 0;
}