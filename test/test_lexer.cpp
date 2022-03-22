//
// Created by Alex on 2022/3/12.
//
#include "lest.hpp"
#include "parser.h"

const lest::test Lexers[] = {
        CASE("Simple Test") {
            const char *Tokens = "a b c 111 22 33 'abc' aa";
            ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
            Lexer.reset(Tokens, Tokens + strlen(Tokens));
            Lexer.dump();
        },
        CASE("Complex Test") {
            const char *News = "import aaa.bbb.ccc;\n"
                                 "int<int, value> main() {\n"
                                 "  a = 1 + 2 * 5;\n"
                                 "}";

            ParserLexer<> Lexer(&LexerStates[0], LexerWhitespaceSymbol);
            Lexer.reset(News, News + strlen(News));
            Lexer.dump();
        },
};

const lest::test Parsers[] = {
        CASE("Parser") {
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
};

int main(int argc, char *argv[]) {
    return lest::run(Parsers, argc, argv, std::cout);
}
