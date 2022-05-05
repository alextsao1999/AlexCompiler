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
                ret(add(IReg, add(ImmOrReg, ImmOrReg)))
                | ret(add(IReg, mul(ImmOrReg, ImmOrReg)));

        SelectContext Ctx;
        auto Res = AddRR(Stack[0], Ctx);

        auto Rule =
                AddRR(Stack[0], Ctx) >> Emit(Pattern::Add, 1, 2);

        //Rule.apply(Stack[0], Ctx, block);
        std::cout << Res;

    }

};


#endif //DRAGON_ARMSELECTOR_H
