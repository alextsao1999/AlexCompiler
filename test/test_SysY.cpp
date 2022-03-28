//
// Created by Alex on 2022/3/12.
//
#include "gtest/gtest.h"
#include "Codegen.h"
#include "parser.h"

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
TEST(Sys, Expr) {

    auto Val = ParseCode(R"(
int main(){
    int a = 0;
    while (a < 20) {
      a = a + 1;
    }
}
)");
    Codegen CG(Context);
    CG.visit(Val);

    auto *Module = CG.getModule();

    Module->dump(std::cout);

}

