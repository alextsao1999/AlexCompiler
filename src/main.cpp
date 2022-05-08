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
#include "LoopSimplify.h"
#include "Codegen.h"
#include "SCCP.h"
#include "Lowering.h"
#include "RISCVLowering.h"
#include "MachineSelect.h"
#include "Liveness.h"
#include "GraphColor.h"
#include "MachineElim.h"
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
    auto Module = compileModule(R"(
        int main(int a, int b){
            int d = 10;
            int x = a + b * 2;
            d = d * 2;
            if (x == 100) { b = 200; }
            int c = x + 2 * b;
            c = c + d;
            return x + c / a;
        }
    )");
/*
    auto Module = compileModule(R"(
        int main(int a, int b){
            int d = 10;
            do {
                d = d + 1;
            } while (0);
            return d;
        }
    )");
*/


    // swap
/*
    auto Module = compileModule(R"(
        int main(int a, int b){
            int x = 10;
            int y = 20;
            do {
                int t = x;
                x = y;
                y = t;
            } while (x == 20);
            int c = x * 2 + y;
            return c;
        }
    )");
*/
    // loss copy
/*
    auto Module = compileModule(R"(
        int main(int a, int b){
            int i = 1;
            int y = 0;
            do {
                y = i;
                i = i + 1;
            } while (i == 2);
            return y + 2;
        }
    )");
*/
/*
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
*/
    auto &Fun = *Module->getFunction("main");
    // Module->dump(std::cout);
    Dominance Dom;
    SSAConstructor Cons;
    GVN GVN;
    LoopAnalyse LA;
    BranchElim BE;
    LoopSimplify LS;
    SCCP SCCP;
    SSADestructor Des;

    Dom.runOnFunction(Fun);
    Cons.runOnFunction(Fun);
    GVN.runOnFunction(Fun);
    Des.runOnFunction(Fun);
    /*
    BE.runOnFunction(Fun);
    Dom.runOnFunction(Fun);
    LA.runOnFunction(Fun);
    LS.runOnFunction(Fun);
    Dom.runOnFunction(Fun);*/
    //SCCP.runOnFunction(Fun);

    Lowering PD;
    MachineSelect AS;
    Liveness LV;
    RISCVLowering RVL;
    GraphColor GC;
    MachineElim ME;
    PD.runOnFunction(Fun);
    //AS.runOnFunction(Fun);
    RVL.runOnFunction(Fun);
    LV.runOnFunction(Fun);

    std::cout << "-------------------------------------------------------" << std::endl;
    GC.runOnFunction(Fun);
    ME.runOnFunction(Fun);

    Module->dump(std::cout);
    return 0;
}