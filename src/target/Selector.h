//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_SELECTOR_H
#define DRAGON_SELECTOR_H

#include "PatternNode.h"
namespace Matcher {
    class SelectContext {

    };
    constexpr auto reg() {
        return [](PatternNode *node, SelectContext &context) {
            if (node->is<RegisterNode>()) {
                return true;
            } else {

            }
            return false;
        };
    }

    constexpr auto Mat = reg();

    constexpr int value() {
        return 0;
    }

    class Selector {
        static void match() {
            /// auto ireg = reg("i32");
            /// auto rewriter = add(ireg, ireg) => add(reg("$1"), reg("$2"))
            SelectContext Ctx;
            Mat(nullptr, Ctx);
        }
    };

} // namespace Matcher
#endif //DRAGON_SELECTOR_H
