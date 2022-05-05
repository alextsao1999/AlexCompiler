//
// Created by Alex on 2022/5/4.
//

#ifndef DRAGON_ARMSELECTOR_H
#define DRAGON_ARMSELECTOR_H

#include "MachinePass.h"
#include "Selector.h"

using namespace Matcher;
class ARMSelector : public MachineBlockPass {
public:
    ARMSelector() {}

    void runOnMachineBlock(MachineBlock &block) override {
        doSelectOnDAG(block);
    }


    void doSelectOnDAG(MachineBlock &block) {
        auto *RootNode = block.getRootNode();

        std::vector<PatternNode *> Stack;
        Stack.insert(Stack.begin(), RootNode->op_begin(), RootNode->op_end());

        constexpr auto ImmOrReg = Imm | IReg;
        constexpr auto AddRR =
                  add(IReg, IReg)
                | add(IReg, Imm)
                | ret(add(IReg, add(IReg, Imm)))
                | ret(add(IReg, mul(ImmOrReg, ImmOrReg)));

        SelectContext Ctx;
        auto Res = AddRR(Stack[0], Ctx);
        std::cout << Res;
    }

};


#endif //DRAGON_ARMSELECTOR_H
