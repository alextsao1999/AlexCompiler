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

};

TEST(Parser, Test) {
    const char *News = "int a = 10;";

    GLRParser<> Parser(false);
    Parser.reset(News, News + strlen(News));
    Parser.parse();
    std::cout << Parser.value().dump(4);
}