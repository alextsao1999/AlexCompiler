//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
int main(int argc, char **argv) {
    Context Context;
    auto M = std::make_unique<Module>("test", Context);

    auto *F = M->createFunction("func1");

    BasicBlock::Create(F, "Entry");
    IRBuilder Builder(F);

    auto *A = Builder.createAlloca(Context.getInt32Ty(), "A");

    auto *Add = Builder.createAdd(Builder.getInt(1), A);
    auto *Mul = Builder.createMul(Add, Builder.getInt(6));
    auto *Load = Builder.createLoad(Mul, "val");
    auto *Ret = Builder.createRet(Load);

    F->dump(std::cout);

    return 0;
}