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
              while (i<20) a = a + i;
              while (a < 50) a = a + 1;
              a = a + 1;
            }
            while (a > 500) a = a - 1;
        }
    )");

    auto *Fun = Module->getFunction("main");

    EXPECT_EQ_VALUE(Fun, R"(
    def main() -> i32 {
        entry.0:    preds=() succs=(%while.header.0)
        %a.0 = alloca i32, 1
        store i32* %a.0, i32 0
        br %while.header.0

        while.header.0:    preds=(%while.leave.2, %entry.0) succs=(%while.body.0, %while.leave.0)
        %load.0 = load i32* %a.0
        %lt.0 = lt i32 %load.0, i32 20
        condbr i32 %lt.0, %while.body.0, %while.leave.0

        while.body.0:    preds=(%while.header.0) succs=(%while.header.1)
        %i.0 = alloca i32, 1
        store i32* %i.0, i32 10
        br %while.header.1

        while.leave.0:    preds=(%while.header.0) succs=(%while.header.3)
        br %while.header.3

        while.header.1:    preds=(%while.body.1, %while.body.0) succs=(%while.body.1, %while.leave.1)
        %load.1 = load i32* %i.0
        %lt.1 = lt i32 %load.1, i32 20
        condbr i32 %lt.1, %while.body.1, %while.leave.1

        while.body.1:    preds=(%while.header.1) succs=(%while.header.1)
        %load.2 = load i32* %a.0
        %load.3 = load i32* %i.0
        %add.0 = add i32 %load.2, i32 %load.3
        store i32* %a.0, i32 %add.0
        br %while.header.1

        while.leave.1:    preds=(%while.header.1) succs=(%while.header.2)
        br %while.header.2

        while.header.2:    preds=(%while.body.2, %while.leave.1) succs=(%while.body.2, %while.leave.2)
        %load.4 = load i32* %a.0
        %lt.2 = lt i32 %load.4, i32 50
        condbr i32 %lt.2, %while.body.2, %while.leave.2

        while.body.2:    preds=(%while.header.2) succs=(%while.header.2)
        %load.5 = load i32* %a.0
        %add.1 = add i32 %load.5, i32 1
        store i32* %a.0, i32 %add.1
        br %while.header.2

        while.leave.2:    preds=(%while.header.2) succs=(%while.header.0)
        %load.6 = load i32* %a.0
        %add.2 = add i32 %load.6, i32 1
        store i32* %a.0, i32 %add.2
        br %while.header.0

        while.header.3:    preds=(%while.body.3, %while.leave.0) succs=(%while.body.3, %while.leave.3)
        %load.7 = load i32* %a.0
        %gt.0 = gt i32 %load.7, i32 500
        condbr i32 %gt.0, %while.body.3, %while.leave.3

        while.body.3:    preds=(%while.header.3) succs=(%while.header.3)
        %load.8 = load i32* %a.0
        %sub.0 = sub i32 %load.8, i32 1
        store i32* %a.0, i32 %sub.0
        br %while.header.3

        while.leave.3:    preds=(%while.header.3) succs=()
        ret
    }
)");

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

    for (auto &Loop: Fun->loops) {
        std::cout << "Loop: ";
        Loop.getHeader()->dumpName(std::cout) << std::endl;
    }

    Fun->dump(std::cout);
}


