#include "test_common.h"

#define EXPECT_EQ_ASM(V, EXPECTED) \
    EXPECT_EQ(SplitAndTrim(compileAsm(V)), SplitAndTrim(EXPECTED))

#undef CHECK_OR_DUMP
#define CHECK_OR_DUMP(V, C) if (isStrEmpty(C)) std::cout << compileAsm(V); else EXPECT_EQ_ASM(V, C);

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
    CHECK_OR_DUMP(Mod.get(), R"(
# fib
fib_entry_0:
    lt t0, a0, 2
    bne t0, zero, fib_if_then_0
    br fib_if_leave_0
fib_if_then_0:
    mv a0, a0
    ret
fib_if_leave_0:
    sub t0, a0, 1
    mv a0, t0
    call fib_entry_0
    mv t0, a0
    sub t1, a0, 2
    mv a0, t1
    call fib_entry_0
    mv t1, a0
    add t0, t0, t1
    mv a0, t0
    ret
)");
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
    CHECK_OR_DUMP(Mod.get(), R"(
# main
main_entry_0:
    add t0, a0, 1
    add t1, a1, 1
    mv a0, t0
    mv a1, t1
    call main_entry_0
    mv t0, a0
    mul t1, a1, 2
    add t1, a0, t1
    mul t0, t0, 2
    ne t2, t1, 100
    bne t2, zero, main_if_then_0
main_split_critial_edge_0:
    mv t2, a1
    br main_if_leave_0
main_if_then_0:
    mv t2, 200
main_if_leave_0:
    mul t2, 2, t3
    add t2, t1, t2
    add t0, t2, t0
    div t0, t0, a0
    add t0, t1, t0
    mv a0, t0
    ret
)");
}

TEST(ASM, Add) {
    auto Mod = compileModule(R"(
        int add(int val1, int val2, int val3, int val4){
            return val1 + val2 + val3 + val4;
        }
)");
    CHECK_OR_DUMP(Mod.get(), R"(
# add
add_entry_0:
    add t0, a0, a1
    add t0, t0, a2
    add t0, t0, a3
    mv a0, t0
    ret
)");
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
    CHECK_OR_DUMP(Mod.get(), R"(
# main
main_entry_0:
    mv t0, 20
    mv t0, 10
main_do_body_0:
main_do_cond_0:
    eq t0, t2, 20
    bne t0, zero, main_split_critial_edge_0
    br main_do_leave_0
main_split_critial_edge_0:
    mv t1, t2
    mv t0, t2
    mv t0, t1
    br main_do_body_0
main_do_leave_0:
    mul t0, t2, 2
    add t0, t0, t2
    mv a0, t0
    ret
)");
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
    CHECK_OR_DUMP(Mod.get(), R"(
# main
main_entry_0:
    mv t0, 1
main_do_body_0:
    add t1, t2, 1
main_do_cond_0:
    eq t0, t1, 2
    bne t0, zero, main_split_critial_edge_0
    br main_do_leave_0
main_split_critial_edge_0:
    mv t0, t1
    br main_do_body_0
main_do_leave_0:
    add t0, t2, 2
    mv a0, t0
    ret
)");
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
    CHECK_OR_DUMP(Mod.get(), R"(
# main
main_entry_0:
    mv t0, 10
main_do_body_0:
    add t0, t1, 1
main_do_cond_0:
    bne zero, zero, main_split_critial_edge_0
    br main_do_leave_0
main_split_critial_edge_0:
    mv t0, t0
    br main_do_body_0
main_do_leave_0:
    mv a0, t0
    ret
)");
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
    CHECK_OR_DUMP(Mod.get(), R"(
# main
main_entry_0:
    mv t0, zero
main_while_header_0:
    lt t0, t1, 20
    bne t0, zero, main_while_body_0
    br main_while_leave_0
main_while_body_0:
    mv t0, 10
    br main_while_header_1
main_while_leave_0:
    mv t0, t1
    br main_while_header_3
main_while_header_1:
    lt t0, t1, 20
    bne t0, zero, main_while_body_1
    br main_while_leave_1
main_while_body_1:
    add t0, t1, 1
    mv t0, t0
    br main_while_header_1
main_while_leave_1:
    mv t0, t1
main_while_header_2:
    lt t0, t1, 50
    bne t0, zero, main_while_body_2
    br main_while_leave_2
main_while_body_2:
    add t0, t1, 1
    mv t0, t0
    br main_while_header_2
main_while_leave_2:
    add t0, t1, 1
    mv t0, t0
    br main_while_header_0
main_while_header_3:
    gt t0, t1, 500
    bne t0, zero, main_while_body_3
    br main_while_leave_3
main_while_body_3:
    sub t0, t1, 1
    mv t0, t0
    br main_while_header_3
main_while_leave_3:
    mv a0, t1
    ret
)");
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
    CHECK_OR_DUMP(Mod.get(), R"(
# add
add_entry_0:
    add t0, a0, a1
    mv a0, t0
    ret

# main
main_entry_0:
    mv a0, 1
    mv a1, 2
    call add_entry_0
    mv t0, a0
    mv a0, t0
    ret
)");
}

