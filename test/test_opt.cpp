//
// Created by Alex on 2022/5/20.
//
#define ENABLE_DUMP 1
#include "test_common.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Dominance.h"
#include "SSAConstructor.h"
#include "Inliner.h"
#include "LoopAnalyse.h"
#include "LoopSimplify.h"
#include "GVN.h"
#include "BranchElim.h"
#include "ADCE.h"

#include "PatternNode.h"
#include "DCE.h"
#include "LICM.h"
#include "SCCP.h"

template<typename ...Passes>
inline std::unique_ptr<Module> compileWithPasses(const char *Code) {
    auto Mod = compileModule(Code);
    PassManager PM;
    PM.addPass(new Dominance);
    PM.addPass(new SSAConstructor);
    (PM.addPass(new Passes), ...);
    PM.run(Mod.get());
    return Mod;
}

template<typename ...Passes>
inline std::string RunPasses(Module *M) {
    PassManager PM;
    (PM.addPass(new Passes), ...);
    PM.run(M);
    return M->dumpToString();
}

TEST(Pass, GVN) {
    auto Mod = compileWithPasses<GVN>("int main() {"
                                      "  int a = 1 + 2;"
                                      "  return a;"
                                      "}");
    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 3
}
)");
}

TEST(Pass, GVN1) {
    auto Mod = compileWithPasses<GVN>("int main() {"
                                      "  int a = 1 + 2;"
                                      "  int b = a + 1;"
                                      "  return b;"
                                      "}");
    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 4
}
)");
}

TEST(Pass, GVN2) {
    auto Mod = compileWithPasses<GVN>("int main() {"
                                      "  int a = 1 + 2;"
                                      "  int b = a + 1;"
                                      "  int c = b + 1;"
                                      "  return c;"
                                      "}");
    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 5
}
)");
}

TEST(Pass, GVN3) {
    auto Mod = compileWithPasses<GVN>("int main() {"
                                      "  int a = 1 + 2;"
                                      "  int b = a + 1;"
                                      "  int c = b + 1;"
                                      "  int d = c + 1;"
                                      "  return d;"
                                      "}");
    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 6
}
)");
}

TEST(Pass, GVN4) {
    auto Mod = compileWithPasses<GVN>("int main() {"
                                      "  int a = 1 + 2;"
                                      "  if (a > 0) {"
                                      "    int b = a + 1;"
                                      "    return b;"
                                      "  } else {"
                                      "    int c = a + 1;"
                                      "    return c;"
                                      "  }"
                                      "}");
    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=(%if.cond.0) doms=(%if.cond.0)
br %if.cond.0

if.cond.0:    preds=(%entry.0) succs=(%if.body.0, %if.else.0) doms=(%if.body.0, %if.else.0) idom=%entry.0
condbr i32 1, %if.body.0, %if.else.0

if.body.0:    preds=(%if.cond.0) succs=() idom=%if.cond.0
ret i32 4

if.else.0:    preds=(%if.cond.0) succs=() idom=%if.cond.0
ret i32 4

if.leave.0:    preds=() succs=()
ret
}
)");
}

TEST(Pass, DCE) {
    auto Mod = compileWithPasses<DCE>("int main() {"
                                      "  int a = 1 + 2;"
                                      "  return 10;"
                                      "}");

    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 10
}
)");
}

TEST(Pass, ADCE) {
    auto Mod = compileWithPasses<ADCE>("int main() {"
                                       "  int a = 1 + 2;"
                                       "  return 10;"
                                       "}");

    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 10
}

)");
}

TEST(Pass, Inline) {
    auto Mod = compileWithPasses<Inliner, Dominance, SSAConstructor, GVN, BranchElim, Dominance>(
            "int main() {"
            "  int a = add(1, 2);"
            "  return a;"
            "}"
            "int add(int a, int b) {"
            "  return a + b;"
            "}");

    CHECK_OR_DUMP(Mod, R"(
Module: Module
def main() -> i32 {
entry.split.0:    preds=() succs=()
ret i32 3
}


def add(i32 %a, i32 %b) -> i32 {
entry.0:    preds=() succs=()
%add.0 = add i32 %a, i32 %b
ret i32 %add.0
}
)");
}

TEST(Pass, SCCP) {
    auto Mod = compileWithPasses<SCCP, Dominance, BranchElim, Dominance>(
            "int test_add(int a, int b) {"
            "  if (a == 10) {"
            "    return a + b + 20;"
            "  } else {"
            "    return a * b + 10;"
            "  }"
            "}"
            ""
            "int main() {"
            "  int a = 1 + 9;"
            "  int c = 20;"
            "  if (a == 10) {"
            "    c = c + 15;"
            "  } else {"
            "    c = c + 13;"
            "  }"
            "  c = test_add(c, a);"
            "  return c;"
            "}");

    CHECK_OR_DUMP(Mod, R"(
Module: Module
def test_add(i32 %a, i32 %b) -> i32 {
entry.0:    preds=() succs=(%if.body.0, %if.else.0) doms=(%if.body.0, %if.else.0)
%eq.0 = eq i32 %a, i32 10
condbr i32 %eq.0, %if.body.0, %if.else.0

if.body.0:    preds=(%entry.0) succs=() idom=%entry.0
%add.0 = add i32 %a, i32 %b
%add.1 = add i32 %add.0, i32 20
ret i32 %add.1

if.else.0:    preds=(%entry.0) succs=() idom=%entry.0
%mul.0 = mul i32 %a, i32 %b
%add.2 = add i32 %mul.0, i32 10
ret i32 %add.2
}

def main() -> i32 {
entry.0:    preds=() succs=()
ret i32 360
}
)");
}
