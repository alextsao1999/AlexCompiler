#include "test_common.h"

inline std::string compileAsm(Module *M) {
    PassManager PM;
    PM.addPass(new Dominance);
    PM.addPass(new SSAConstructor);
    PM.addPass(new GVN);
    PM.addPass(new SSADestructor);
    PM.addPass(new Lowering);
    PM.addPass(new RISCVLowering);
    PM.addPass(new Liveness);
    PM.addPass(new GraphColor);
    PM.addPass(new MachineElim);
    PM.run(M);
    RISCVEmit E;
    E.run(M);
    return E.str();
}

TEST(ASM, Simple) {
    auto Mod = compileModule(R"(
        int fib(int t){
            if(t < 2) return t;
            return fib(t-1) + fib(t-2);
        }
)");
    std::cout << compileAsm(Mod.get());
}

TEST(ASM, Expr1) {
    auto Mod = compileModule(R"(
        int main(int a, int b){
            int d = main(a+1, b+1);
            int x = a + b * 2;
            d = d * 2;
            if (x != 100) { b = 200; }
            int c = x + 2 * b;
            c = c + d;
            return x + c / a;
        }
)");
    std::cout << Mod->dumpToString() << std::endl;
    std::cout << compileAsm(Mod.get());
}

TEST(ASM, Add) {
    auto Mod = compileModule(R"(
        int add(int val1, int val2, int val3, int val4){
            return val1 + val2 + val3 + val4;
        }
)");
    std::cout << compileAsm(Mod.get());
}

TEST(ASM, Swap) {
    auto Mod = compileModule(R"(
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
    std::cout << compileAsm(Mod.get());
}

TEST(ASM, LostCopy) {
    auto Mod = compileModule(R"(
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
    std::cout << compileAsm(Mod.get());
}

TEST(ASM, LoopTest1) {
    auto Mod = compileModule(R"(
        int main(int a, int b){
            int d = 10;
            do {
                d = d + 1;
            } while (0);
            return d;
        }
)");
    std::cout << compileAsm(Mod.get());
}

TEST(ASM, LoopTest2) {
    auto Mod = compileModule(R"(
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
    std::cout << compileAsm(Mod.get());
}


TEST(ASM, Call) {
    auto Mod = compileModule(R"(
        int add(int x, int y) {
            return x + y;
        }

        int main(){
            return add(1, 2);
        }
)");
    std::cout << compileAsm(Mod.get());
}

