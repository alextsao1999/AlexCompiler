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
#include "IDFCalculator.h"
#include "LoopAnalyse.h"
#include "Codegen.h"
static Context Context;

value_t ParseCode(const char *str) {
    LRParser<> Parser(false);
    Parser.reset(str, str + strlen(str));
    Parser.parse();
    if (!Parser.accept()) {
        return value_t();
    }
    return Parser.value();
}

std::unique_ptr<Module> compileModule(const char *str) {
    auto Val = ParseCode(str);
    Codegen CG(Context);
    CG.visit(Val);
    return std::move(CG.getModule());
}

int main(int argc, char **argv) {
    auto Module = compileModule(R"(
        int main(){
            int a = 0;
            while (a < 20) {
              int i = 10;
              while (i<20) a = a + i;
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
    BranchElim BE;

    Dom.runOnFunction(Fun);
    Cons.runOnFunction(Fun);
    GVN.runOnFunction(Fun);
    BE.runOnFunction(Fun);
    LA.runOnFunction(Fun);

    Module->dump(std::cout);

    return 0;
}