//
// Created by Alex on 2022/3/16.
//

#ifndef DRAGON_DCE_H
#define DRAGON_DCE_H

#include "PassManager.h"
#include "Function.h"

class DCE : public FunctionPass {
public:
    void runOnFunction(Function *function) override {


    }
};


#endif //DRAGON_DCE_H
