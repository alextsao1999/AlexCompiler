//
// Created by Alex on 2022/5/4.
//

#ifndef DRAGON_MACHINESELECT_H
#define DRAGON_MACHINESELECT_H

#include "MachinePass.h"
#include "Selector.h"

using namespace Matcher;
class MachineSelect : public MachineBlockPass {
public:
    MachineSelect() {}

    void runOnMachineBlock(MachineBlock &block) override {
        doSelectOnDAG(block);
    }

    virtual bool doApplyRule(PatternNode *node, SelectContext &ctx, MachineBlock &block) {
        constexpr auto ImmOrReg = Imm | IReg;
        constexpr auto RegAlloc = sel<PatternNode>([](PatternNode *node, SelectContext &ctx) {
            if (node->getType() == ctx.getContext()->getInt32Ty()) {
                node->replaceWith(new RegisterNode(node));
                return true;
            }
            return false;
        });
        constexpr auto Rule =
                add(IReg, Imm)
                | add(IReg, IReg)
                | sub(IReg, Imm)
                | sub(IReg, same(0))
                | ret(RegAlloc);

        /*constexpr auto Rule =
                add(IReg, Imm) > Emit(Pattern::Add, 0, 2)
                || ret(add(IReg, add(IReg, Imm))) > Emit(Pattern::Add, 0, 3)
                || ret(sub(IReg, IReg)) > Emit(Pattern::Add, 0, 4)
                || add(IReg, mul(ImmOrReg, ImmOrReg)) > Emit(Pattern::Add, 0, 5);*/

        auto Res = Rule(node, ctx);

        return Res;
    }

    void doSelectOnDAG(MachineBlock &block) {
        auto *RootNode = block.getRootNode();

        std::vector<PatternNode *> Stack;
        Stack.insert(Stack.begin(), RootNode->op_begin(), RootNode->op_end());

        SelectContext Ctx(block.getOrigin()->getContext());
        while (!Stack.empty()) {
            auto *Node = Stack.back();
            Stack.pop_back();

            if (doApplyRule(Node, Ctx, block)) {
                Stack.insert(Stack.end(), Ctx.begin(), Ctx.end());
                Stack.clear();
            } else {
                std::cout << "Couldn't select node" << std::endl;
            }
        }
        //Rule.apply(Stack[0], Ctx, block);
    }

};


#endif //DRAGON_MACHINESELECT_H
