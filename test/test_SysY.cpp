//
// Created by Alex on 2022/3/12.
//
#include "test_common.h"
#include "Codegen.h"
#include "SSAConstructor.h"
#include "Dominance.h"
#include "LoopAnalyse.h"
#include "parser.h"
#include "GVN.h"
#include "BranchElim.h"
#include "LoopSimplify.h"

value_t ParseCode(const char *str) {
    LRParser<> Parser(false);
    Parser.reset(str, str + strlen(str));
    Parser.parse();
    if (!Parser.accept()) {
        return value_t();
    }
    return Parser.value();
}
Context Context;

std::unique_ptr<Module> compileModule(const char *str) {
    auto Val = ParseCode(str);
    Codegen CG(Context);
    CG.visit(Val);
    return std::move(CG.getModule());
}

TEST(Sys, Expr) {
    auto Module = compileModule(R"(
        int main(){
            int a = 0;
            while (a < 20) {
              int i = 10;
              while (i<20) i = i + 1;
              while (a < 50) a = a + 1;
              a = a + 1;
            }
            while (a > 500) a = a - 1;
            return a;
        }
    )");

    auto *Fun = Module->getFunction("main");


    Dominance Dom;
    SSAConstructor Cons;
    GVN GVN;
    LoopAnalyse LA;
    LoopSimplify LS;
    BranchElim BE;

    Dom.runOnFunction(Fun);
    Cons.runOnFunction(Fun);
    GVN.runOnFunction(Fun);
    BE.runOnFunction(Fun);
    Dom.runOnFunction(Fun);
    LA.runOnFunction(Fun);
    //LS.runOnFunction(Fun);
    Dom.runOnFunction(Fun);

    for (auto &Loop: Fun->loops) {
        std::cout << "Loop: ";
        Loop.getHeader()->dumpName(std::cout) << std::endl;
    }

    Fun->dump(std::cout);
}


