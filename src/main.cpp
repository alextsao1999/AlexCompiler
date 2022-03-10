//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
int main(int argc, char **argv) {
    auto *M = new Module("test");

    auto *F = M->createFunction("func1");

    auto *BB = BasicBlock::Create(F, "Entry");

    IRBuilder Builder(F);

    auto *Add = Builder.createAdd(Builder.getInt(1), Builder.getInt(2));
    auto *Mul = Builder.createMul(Add, Builder.getInt(6));
    auto *Load = Builder.createLoad(Mul, "val");
    auto *Ret = Builder.createRet(Load);

    F->dump(std::cout);

    delete M;

    return 0;
}