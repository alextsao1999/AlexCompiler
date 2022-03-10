//
// Created by Alex on 2021/10/22.
//

#ifndef DRAGONCOMPILER_COMMON_H
#define DRAGONCOMPILER_COMMON_H

#include <iostream>

#define assert(x) if (!(x)) { std::cout << "Assertion failed: " << x << " " << __FILE__ << ":" << __LINE__ << std::endl; }
#define unreachable() assert(!"Unreachable here!");

#endif //DRAGONCOMPILER_COMMON_H
