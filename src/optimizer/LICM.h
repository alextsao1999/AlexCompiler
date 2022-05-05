//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_LICM_H
#define DRAGON_LICM_H

/**
 * Loop invariant code motion.
 */

#include "PassManager.h"
#include "Function.h"

class LICM : public FunctionPass {
public:
    void runOnFunction(Function &function) override {

    }
};


#endif //DRAGON_LICM_H
