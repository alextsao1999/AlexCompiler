//
// Created by Alex on 2021/10/22.
//

#ifndef DRAGONCOMPILER_COMMON_H
#define DRAGONCOMPILER_COMMON_H

#include <iostream>
#include <algorithm>
using StrView = std::string_view;

inline std::string strlower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

#define ASSERT(x) if (!(x)) { std::cout << "Assertion failed: " << (x) << " " << __FILE__ << ":" << __LINE__ << std::endl;abort(); }
#define UNREACHEABLE() ASSERT(!"Unreachable here!");
#define debug(...) fprintf(stderr, __VA_ARGS__)

#endif //DRAGONCOMPILER_COMMON_H
