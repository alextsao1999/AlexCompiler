//
// Created by Alex on 2022/5/20.
//
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
#include "SCCP.h"

#include "PatternNode.h"
#include "DCE.h"
#include "LICM.h"

template<typename PassTy>
inline std::string TestPass(Module *M) {
    PassManager PM;
    PM.addPass(new Dominance);
    PM.addPass(new SSAConstructor);
    PM.addPass(new PassTy);
    PM.run(M);
    std::stringstream ss;
    M->dump(ss);
    return ss.str();
}

template<typename ...Passes>
inline std::string TestPasses(Module *M) {
    PassManager PM;
    (PM.addPass(new Passes), ...);
    PM.run(M);
    std::stringstream ss;
    M->dump(ss);
    return ss.str();
}

TEST(Pass, GVN) {
    auto Mod = compileModule("int main() {"
                             "  int a = 1 + 2;"
                             "  return a;"
                             "}");
    std::cout << TestPass<GVN>(Mod.get());

    auto Mod2 = compileModule("int main() {"
                              "  int a = 1 + 2;"
                              "  int b = a + 1;"
                              "  return b;"
                              "}");
    std::cout << TestPass<GVN>(Mod2.get());

    auto Mod3 = compileModule("int main() {"
                              "  int a = 1 + 2;"
                              "  int b = a + 1;"
                              "  int c = b + 1;"
                              "  return c;"
                              "}");
    std::cout << TestPass<GVN>(Mod3.get());

    auto Mod4 = compileModule("int main() {"
                              "  int a = 1 + 2;"
                              "  int b = a + 1;"
                              "  int c = b + 1;"
                              "  int d = c + 1;"
                              "  return d;"
                              "}");
    std::cout << TestPass<GVN>(Mod4.get());

}

TEST(Pass, DCE) {
    auto Mod = compileModule("int main() {"
                             "  int a = 1 + 2;"
                             "  return 10;"
                             "}");
    auto *F = Mod->getFunction();
    std::cout << TestPass<DCE>(Mod.get());
}

TEST(Pass, ADCE) {
    auto Mod = compileModule("int main() {"
                             "  int a = 1 + 2;"
                             "  return 10;"
                             "}");
    std::cout << TestPass<ADCE>(Mod.get());
}

TEST(Pass, Inline) {
    auto Mod = compileModule("int main() {"
                             "  int a = add(1, 2);"
                             "  return a;"
                             "}"
                             "int add(int a, int b) {"
                             "  return a + b;"
                             "}");

    std::cout << TestPasses<
            Dominance,
            SSAConstructor,
            Inliner,
            Dominance,
            SSAConstructor,
            GVN,
            BranchElim,
            Dominance
    >(Mod.get());
}

TEST(Pass, LICM) {
    auto Mod = compileModule("int main() {"
                             "  int a = 0;"
                             "  int b = 40;"
                             "  while(a < 20) {"
                             "    a = b * 20 + 1;"
                             "  }"
                             "  return a;"
                             "}");

    std::cout << TestPasses<
            Dominance,
            SSAConstructor,
            Inliner,
            Dominance,
            SSAConstructor,
            //GVN,
            BranchElim,
            LoopAnalyse,
            LoopSimplify,
            LICM,
            Dominance
    >(Mod.get());
}