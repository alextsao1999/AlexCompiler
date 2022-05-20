//
// Created by Alex on 2022/3/12.
//
#include "gtest/gtest.h"
#include "parser.h"

TEST(Lexer, Regex) {
    const char *Tokens = "a b c 111 22 33 aa int bool test ( ) { } + - ";
    ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    Lexer.reset(Tokens, Tokens + strlen(Tokens));
    Lexer.dump();
}

TEST(Lexer, FloatLiteral) {
    const char *Tokens = "1.2f .2f 0.2f 1.2e2f 1.2e-2f 1.2e+2f";
    ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    Lexer.reset(Tokens, Tokens + strlen(Tokens));
    Lexer.dump();
}

TEST(Lexer, Keywords) {
    const char *Tokens = "int bool return if else while do";
    ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    Lexer.reset(Tokens, Tokens + strlen(Tokens));
    Lexer.dump();
}

TEST(Lexer, Complex) {
    const char *News = "int main() {\n"
                       "  int a = 1 + 2 * 5;\n"
                       "  return a;"
                       "}";

    ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
    Lexer.reset(News, News + strlen(News));
    Lexer.dump();
}

