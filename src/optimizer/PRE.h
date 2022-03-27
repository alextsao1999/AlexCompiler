//
// Created by Alex on 2022/3/24.
//

#ifndef DRAGON_PRE_H
#define DRAGON_PRE_H

#include "PassManager.h"

struct PhiState {

};

/**
 * The class performs the Partial Redundancy Elimination (PRE) optimization
 */
class PRE : public FunctionPass {
public:
    void runOnFunction(Function *function) override {

    }
};


#endif //DRAGON_PRE_H
