//
// Created by Alex on 2022/3/29.
//

#ifndef DRAGON_TEST_COMMON_H
#define DRAGON_TEST_COMMON_H

#define ENABLE_DUMP 0

#include "gtest/gtest.h"

#include "parser.h"
#include "Codegen.h"
#include "SSAConstructor.h"
#include "Dominance.h"
#include "LoopAnalyse.h"
#include "parser.h"
#include "GVN.h"
#include "BranchElim.h"
#include "LoopSimplify.h"
#include "SSADestructor.h"
#include "GraphColor.h"
#include "Lowering.h"
#include "MachineSelect.h"
#include "Liveness.h"
#include "MachineElim.h"
#include "RISCVLowering.h"
#include "RISCVEmit.h"

inline auto SplitAndTrim(const std::string &str) -> std::string {
    std::vector<std::string> Res;
    std::stringstream SS(str);
    std::string Item;
    while (std::getline(SS, Item)) {
        Res.push_back(Item);
    }
    static const char *TrimChars = "  \t\r\n";
    // Trim Each Line
    for (auto &Line: Res) {
        Line.erase(Line.find_last_not_of(TrimChars) + 1);
        Line.erase(0, Line.find_first_not_of(TrimChars));
    }
    // Remove Empty Line
    Res.erase(std::remove_if(Res.begin(), Res.end(), [](const std::string &str) {
        return str.empty();
    }), Res.end());
    // Join the Res
    std::stringstream SSRes;
    for (auto &Line: Res) {
        SSRes << Line << "\n";
    }
    return SSRes.str();
};

inline value_t ParseCode(const char *str) {
    LRParser<> Parser(false);
    Parser.reset(str, str + strlen(str));
    Parser.parse();
    if (!Parser.accept()) {
        return value_t();
    }
    return Parser.value();
}

Context Context;

inline std::unique_ptr<Module> compileModule(const char *str) {
    auto Val = ParseCode(str);
    Codegen CG(Context);
    CG.visit(Val);
    return std::move(CG.getModule());
}


#define EXPECT_EQ_VALUE(V, EXPECTED) \
    EXPECT_EQ(SplitAndTrim(V->dumpToString()), SplitAndTrim(EXPECTED))

inline bool isStrEmpty(const char *str) {
    return str == nullptr || str[0] == '\0';
}

#if ENABLE_DUMP
#define CHECK_OR_DUMP(V, C) if (isStrEmpty(C)) V->dump(std::cout); else EXPECT_EQ_VALUE(V, C);
#else
#define CHECK_OR_DUMP(V, C) EXPECT_EQ_VALUE(V, C)
#endif

#endif //DRAGON_TEST_COMMON_H
