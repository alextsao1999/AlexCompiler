//
// Created by Alex on 2022/3/29.
//

#ifndef DRAGON_TEST_COMMON_H
#define DRAGON_TEST_COMMON_H

#include "gtest/gtest.h"

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

#define EXPECT_EQ_VALUE(V, EXPECTED) \
    EXPECT_EQ(SplitAndTrim(V->dumpToString()), SplitAndTrim(EXPECTED))

#endif //DRAGON_TEST_COMMON_H
