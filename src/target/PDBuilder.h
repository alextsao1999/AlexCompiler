//
// Created by Alex on 2022/3/13.
//

#ifndef DRAGON_PDBUILDER_H
#define DRAGON_PDBUILDER_H

#include "Function.h"
///< Pattern DAG Builder
class PDBuilder {
public:
    ///< Build a pattern DAG for a function
    BasicBlock *block = nullptr;
    PDBuilder(BasicBlock *block) : block(block) {}

    ///< Build a pattern DAG for basicblock
    void build() {

    }
};

#endif //DRAGON_PDBUILDER_H
