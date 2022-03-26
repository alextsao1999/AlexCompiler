//
// Created by Alex on 2022/3/12.
//
#include "gtest/gtest.h"
#include "parser.h"

TEST(Lexer, Regex) {
    const char *Tokens = "a b c 111 22 33 'abc' aa";
    ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    Lexer.reset(Tokens, Tokens + strlen(Tokens));
    Lexer.dump();
}

TEST(Lexer, Complex) {
    const char *News = "import aaa.bbb.ccc;\n"
                       "int<int, value> main() {\n"
                       "  a = 1 + 2 * 5;\n"
                       "}";

    ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    Lexer.reset(News, News + strlen(News));
    Lexer.dump();
}

#include <unordered_map>
struct MyVisitor : public Visitor<MyVisitor> {
    std::unordered_map<std::string, std::vector<std::reference_wrapper<value_t>>> map;
    void visitProgram(Program value) override {
        visit(value.getValue());
    }
    void visitBlockStmt(BlockStmt value) override {
        visit(value.getValue());
    }
    void visitExprStmt(ExprStmt value) override {
        visit(value.getValue());
    }
    void visitAssignExpr(AssignExpr value) override {
        visit(value.getLeft());
        visit(value.getRight());
    }
    void visitVariableExpr(VariableExpr value) override {
        //std::cout << value.getName();
    }

    void visitBinaryExpr(BinaryExpr value) override {
        visit(value.getLeft());
        visit(value.getRight());
    }

    void visitInvokeExpr(InvokeExpr value) override {
        std::string Name = value.getName();
        map[Name].emplace_back(value);
    }

    void visitFunctionDeclare(FunctionDeclare value) override {
        visit(value.getBlock());
    }

    void visitParamDef(ParamDef value) override {

    }

};

TEST(Parser, Test) {
    const char *News = "import a.c;\n"
                       "fun test1() {"
                       "  test2();"
                       "}"
                       "fun test2() {"
                       ""
                       "}";

    GLRParser<> Parser(false);
    Parser.reset(News, News + strlen(News));
    Parser.parse();
    std::cout << Parser.value().dump(4);
}