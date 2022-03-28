//
// Created by Alex on 2022/3/12.
//
#include "gtest/gtest.h"
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

TEST(Sys, Expr) {

    std::cout << ParseCode(R"(
//test local var define
int main(){
    int a, b0, _c;
    a = 1;
    b0 = 2;
    _c = 3;
    return b0 + _c;
}
)").dump(4);



}

