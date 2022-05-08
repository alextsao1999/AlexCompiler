//
// Created by Alex on 2022/3/12.
//
#include "test_common.h"

TEST(SysExpr, Common) {
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
    CHECK_OR_DUMP(Fun, R"(
def main() -> i32 {
entry.0:    preds=() succs=(%while.header.0)
%a.0 = alloca i32
store i32* %a.0, i32 0
br %while.header.0

while.header.0:    preds=(%while.leave.2, %entry.0) succs=(%while.body.0, %while.leave.0)
%load.0 = load i32* %a.0
%lt.0 = lt i32 %load.0, i32 20
condbr i32 %lt.0, %while.body.0, %while.leave.0

while.body.0:    preds=(%while.header.0) succs=(%while.header.1)
%i.0 = alloca i32
store i32* %i.0, i32 10
br %while.header.1

while.leave.0:    preds=(%while.header.0) succs=(%while.header.3)
br %while.header.3

while.header.1:    preds=(%while.body.1, %while.body.0) succs=(%while.body.1, %while.leave.1)
%load.1 = load i32* %i.0
%lt.1 = lt i32 %load.1, i32 20
condbr i32 %lt.1, %while.body.1, %while.leave.1

while.body.1:    preds=(%while.header.1) succs=(%while.header.1)
%load.2 = load i32* %i.0
%add.0 = add i32 %load.2, i32 1
store i32* %i.0, i32 %add.0
br %while.header.1

while.leave.1:    preds=(%while.header.1) succs=(%while.header.2)
br %while.header.2

while.header.2:    preds=(%while.body.2, %while.leave.1) succs=(%while.body.2, %while.leave.2)
%load.3 = load i32* %a.0
%lt.2 = lt i32 %load.3, i32 50
condbr i32 %lt.2, %while.body.2, %while.leave.2

while.body.2:    preds=(%while.header.2) succs=(%while.header.2)
%load.4 = load i32* %a.0
%add.1 = add i32 %load.4, i32 1
store i32* %a.0, i32 %add.1
br %while.header.2

while.leave.2:    preds=(%while.header.2) succs=(%while.header.0)
%load.5 = load i32* %a.0
%add.2 = add i32 %load.5, i32 1
store i32* %a.0, i32 %add.2
br %while.header.0

while.header.3:    preds=(%while.body.3, %while.leave.0) succs=(%while.body.3, %while.leave.3)
%load.6 = load i32* %a.0
%gt.0 = gt i32 %load.6, i32 500
condbr i32 %gt.0, %while.body.3, %while.leave.3

while.body.3:    preds=(%while.header.3) succs=(%while.header.3)
%load.7 = load i32* %a.0
%sub.0 = sub i32 %load.7, i32 1
store i32* %a.0, i32 %sub.0
br %while.header.3

while.leave.3:    preds=(%while.header.3) succs=()
%load.8 = load i32* %a.0
ret i32 %load.8
}
)");
}

TEST(SysExpr, Add) {
    auto Module = compileModule(R"(
        int main() {
            int a = 0;
            a = a + 1;
            a = a + 2;
            a = a + 3;
            a = a + 4;
            a = a + 5;
            a = a + 6;
            return a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main() -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 0
%load.0 = load i32* %a.0
%add.0 = add i32 %load.0, i32 1
store i32* %a.0, i32 %add.0
%load.1 = load i32* %a.0
%add.1 = add i32 %load.1, i32 2
store i32* %a.0, i32 %add.1
%load.2 = load i32* %a.0
%add.2 = add i32 %load.2, i32 3
store i32* %a.0, i32 %add.2
%load.3 = load i32* %a.0
%add.3 = add i32 %load.3, i32 4
store i32* %a.0, i32 %add.3
%load.4 = load i32* %a.0
%add.4 = add i32 %load.4, i32 5
store i32* %a.0, i32 %add.4
%load.5 = load i32* %a.0
%add.5 = add i32 %load.5, i32 6
store i32* %a.0, i32 %add.5
%load.6 = load i32* %a.0
ret i32 %load.6
}
)");
}

TEST(SysExpr, Sub) {
    auto Module = compileModule(R"(
        int main() {
            int a = 0;
            a = a - 1;
            a = a - 2;
            a = a - 3;
            a = a - 4;
            a = a - 5;
            a = a - 6;
            return a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main() -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 0
%load.0 = load i32* %a.0
%sub.0 = sub i32 %load.0, i32 1
store i32* %a.0, i32 %sub.0
%load.1 = load i32* %a.0
%sub.1 = sub i32 %load.1, i32 2
store i32* %a.0, i32 %sub.1
%load.2 = load i32* %a.0
%sub.2 = sub i32 %load.2, i32 3
store i32* %a.0, i32 %sub.2
%load.3 = load i32* %a.0
%sub.3 = sub i32 %load.3, i32 4
store i32* %a.0, i32 %sub.3
%load.4 = load i32* %a.0
%sub.4 = sub i32 %load.4, i32 5
store i32* %a.0, i32 %sub.4
%load.5 = load i32* %a.0
%sub.5 = sub i32 %load.5, i32 6
store i32* %a.0, i32 %sub.5
%load.6 = load i32* %a.0
ret i32 %load.6
}

)");

}

TEST(SysExpr, Mul) {
    auto Module = compileModule(R"(
        int main() {
            int a = 0;
            a = a * 1;
            a = a * 2;
            a = a * 3;
            a = a * 4;
            a = a * 5;
            a = a * 6;
            return a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main() -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 0
%load.0 = load i32* %a.0
%mul.0 = mul i32 %load.0, i32 1
store i32* %a.0, i32 %mul.0
%load.1 = load i32* %a.0
%mul.1 = mul i32 %load.1, i32 2
store i32* %a.0, i32 %mul.1
%load.2 = load i32* %a.0
%mul.2 = mul i32 %load.2, i32 3
store i32* %a.0, i32 %mul.2
%load.3 = load i32* %a.0
%mul.3 = mul i32 %load.3, i32 4
store i32* %a.0, i32 %mul.3
%load.4 = load i32* %a.0
%mul.4 = mul i32 %load.4, i32 5
store i32* %a.0, i32 %mul.4
%load.5 = load i32* %a.0
%mul.5 = mul i32 %load.5, i32 6
store i32* %a.0, i32 %mul.5
%load.6 = load i32* %a.0
ret i32 %load.6
}
)");

}

TEST(SysExpr, Div) {
    auto Module = compileModule(R"(
        int main() {
            int a = 0;
            a = a / 1;
            a = a / 2;
            a = a / 3;
            a = a / 4;
            return a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main() -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 0
%load.0 = load i32* %a.0
%div.0 = div i32 %load.0, i32 1
store i32* %a.0, i32 %div.0
%load.1 = load i32* %a.0
%div.1 = div i32 %load.1, i32 2
store i32* %a.0, i32 %div.1
%load.2 = load i32* %a.0
%div.2 = div i32 %load.2, i32 3
store i32* %a.0, i32 %div.2
%load.3 = load i32* %a.0
%div.3 = div i32 %load.3, i32 4
store i32* %a.0, i32 %div.3
%load.4 = load i32* %a.0
ret i32 %load.4
}
)");

}

TEST(SysExpr, Mod) {
    auto Module = compileModule(R"(
        int main() {
            int a = 0;
            a = a % 1;
            a = a % 2;
            a = a % 3;
            a = a % 4;
            return a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main() -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 0
%load.0 = load i32* %a.0
%rem.0 = rem i32 %load.0, i32 1
store i32* %a.0, i32 %rem.0
%load.1 = load i32* %a.0
%rem.1 = rem i32 %load.1, i32 2
store i32* %a.0, i32 %rem.1
%load.2 = load i32* %a.0
%rem.2 = rem i32 %load.2, i32 3
store i32* %a.0, i32 %rem.2
%load.3 = load i32* %a.0
%rem.3 = rem i32 %load.3, i32 4
store i32* %a.0, i32 %rem.3
%load.4 = load i32* %a.0
ret i32 %load.4
}

)");

}

TEST(SysExpr, Eq) {
    auto Module = compileModule(R"(
        int main(int a) {
            return a == 1;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main(i32 %a) -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 %a
%load.0 = load i32* %a.0
%eq.0 = eq i32 %load.0, i32 1
ret i32 %eq.0
}
)");
}

TEST(SysExpr, Ne) {
    auto Module = compileModule(R"(
        int main(int a) {
            return a != 1;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main(i32 %a) -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 %a
%load.0 = load i32* %a.0
%ne.0 = ne i32 %load.0, i32 1
ret i32 %ne.0
}
)");

}

TEST(SysExpr, Lt) {
    auto Module = compileModule(R"(
        int main(int a) {
            return a < 1;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main(i32 %a) -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 %a
%load.0 = load i32* %a.0
%lt.0 = lt i32 %load.0, i32 1
ret i32 %lt.0
}
)");
}

TEST(SysExpr, Le) {
    auto Module = compileModule(R"(
        int main(int a) {
            return a <= 1;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main(i32 %a) -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 %a
%load.0 = load i32* %a.0
%lt.0 = lt i32 %load.0, i32 1
ret i32 %lt.0
}
)");

}

TEST(SysExpr, UnaryNeg) {
    auto Module = compileModule(R"(
        int main(int a) {
            return -a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main(i32 %a) -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 %a
%load.0 = load i32* %a.0
%neg.0 = neg i32 %load.0
ret i32 %neg.0
}
)");
}

TEST(SysExpr, UnaryNot) {
    auto Module = compileModule(R"(
        int main(int a) {
            return !a;
        }
    )");
    auto *Fun = Module->getFunction("main");
    CHECK_OR_DUMP(Fun, R"(
def main(i32 %a) -> i32 {
entry.0:    preds=() succs=()
%a.0 = alloca i32
store i32* %a.0, i32 %a
%load.0 = load i32* %a.0
%not.0 = not i32 %load.0
ret i32 %not.0
}
)");

}

