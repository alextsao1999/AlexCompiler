//
// Created by Alex on 2022/5/22.
//

#ifndef DRAGON_OSR_H
#define DRAGON_OSR_H

#include "PassManager.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "GVN.h"

struct SCCInfo {
    unsigned num = 0;
    unsigned low = 0;
    bool instack = false;
    bool visited = false;
    Instruction *header = nullptr;
};

struct ReduceInfo {
    Opcode opcode = OpcodeNop;
    Value *IV = nullptr;
    Value *RC = nullptr;
    ReduceInfo *outer = nullptr;

    ReduceInfo(Opcode opcode, Value *iv, Value *rc, ReduceInfo *outer) : opcode(opcode), IV(iv), RC(rc), outer(outer) {}
    ReduceInfo(Opcode opcode, Value *iv, Value *rc) : opcode(opcode), IV(iv), RC(rc) {}
};

class OSR : public FunctionPass {
public:
    std::map<Instruction *, SCCInfo> mapSCCInfo;
    std::vector<Instruction *> stack;
    std::vector<Instruction *> group;
    std::vector<Instruction *> worklist;

    /***
     * 归纳变量属于在SSA图中的一个强连通分量(SCC), 即忽略掉下面的a_1, 存在一个环路
     * e.g.
     * a_0 = phi(a_1, a_2);
     * t_0 = a_0 + 1;
     * a_2 = t_0;
     */
    unsigned nextNum = 0;
    void DFS(Instruction *Inst) {
        auto &Info = mapSCCInfo[Inst];
        if (Info.visited) {
            return;
        }
        Info.num = nextNum++;
        Info.low = Info.num;
        Info.visited = true;
        Info.instack = true;
        stack.push_back(Inst);
        for (auto &Op : Inst->operands()) {
            if (auto *Var = Op->as<Instruction>()) {
                auto &Cur = mapSCCInfo[Var];
                if (!Cur.visited) {
                    DFS(Var);
                    Info.low = std::min(Cur.low, Info.low);
                }
                // 发现先前已经在栈中的子图的num比自己小, 设置当前的low为栈中的num, 表示分在了同一组中
                if (Cur.instack && Cur.num < Info.num) {
                    Info.low = std::min(Cur.num, Info.low);
                }
            }
        }

        if (Info.num == Info.low) {
            Instruction *Cur;
            do {
                Cur = stack.back();
                stack.pop_back();
                group.push_back(Cur);
                mapSCCInfo[Cur].header = Inst;
                mapSCCInfo[Cur].instack = false;
            } while (Cur != Inst);
            classify();
            group.clear();
        }
    }

    void classify() {
        //dump();
        if (group.size() == 1) {
            auto &Cur = group.back();
            Value *IV, *RC;
            if (isCandidateOperation(Cur, IV, RC)) {
                worklist.push_back(Cur);
                mapSCCInfo[Cur].header = getOperandHeader(IV);
            } else {
                mapSCCInfo[Cur].header = nullptr;
            }
        } else {
            collect();
        }
    }

    /**
     * 收集归纳变量
     * 1. phi只能有1个
     * 2. 只能包含加法减法操作, 并且减法的右操作数不能是归纳变量
     * 3. 加法/减法的另外一个操作数必须是循环不变量
     */
    void collect() {
        Instruction *header = nullptr;
        bool isInd = true;
        for (auto &item : group) {
            auto &Cur = mapSCCInfo[item];
            switch (item->getOpcode()) {
                case OpcodePhi:
                    if (!header) {
                        header = item;
                    } else {
                        isInd = false;
                        goto leave;
                    }
                case OpcodeBinary:
                    assert(item->getOperandNum() > 1);
                    if (getOperandHeader(item->getOperand(1)) == Cur.header) {
                        // i = c - i 这种不属于归纳变量
                        isInd = false;
                        goto leave;
                    }
                    break;
                default:
                    isInd = false;
                    goto leave;
            }
        }
        leave:
        if (!isInd) {
            for (auto &Inst : group) {
                Value *IV, *RC;
                if (isCandidateOperation(Inst, IV, RC)) {
                    worklist.push_back(Inst);
                    doReplace(Inst, IV, RC);
                    mapSCCInfo[Inst].header = getOperandHeader(IV);
                } else {
                    mapSCCInfo[Inst].header = nullptr;
                }
            }
        }
    }

    /**
     * 是否为归纳变量
     */
    bool isIV(Value *Op) {
        auto *Var = Op->as<Instruction>();
        if (Var) {
            auto &Info = mapSCCInfo[Var];
            return Info.header;
        }
        return false;
    }

    /**
     * 是否为区域常量/循环不变量
     */
    bool isRC(Value *Op, Instruction *Inst) {
        // Op定义所在的CFG严格支配IV的header所在的CFG即为区域常量
        if (auto *Var = Op->as<Instruction>()) {
            auto *Def = Var->getParent();
            auto *Cur = Inst->getParent();
            assert(Def && Cur);
            // FIXME: problem?
            return Def->dominates(Cur);
        } else if (Op->as<Constant>()) {
            return true;
        }
        return false;
    }

    /**
     * 是否为候选操作, 具有以下形式的操作是候选操作
     * x = IV x RC
     * x = RC x IV
     * x = RC + IV
     * x = IV ± RC
     */
    bool isCandidateOperation(Instruction *Inst, Value *&IV, Value *&RC) {
        if (Inst->getOperandNum() < 2) {
            return false;
        }
        Value *Op1 = Inst->getOperand(0);
        Value *Op2 = Inst->getOperand(1);

        if (auto *Bin = Inst->as<BinaryInst>()) {
            if (Bin->getOp() == BinaryOp::Mul || Bin->getOp() == BinaryOp::Add) {
                if (isIV(Op1) && isRC(Op2, getOperandHeader(Op1))) {
                    IV = Op1;
                    RC = Op2;
                    return true;
                }
                if (isIV(Op2) && isRC(Op1, getOperandHeader(Op2))) {
                    IV = Op2;
                    RC = Op1;
                    return true;
                }
            }
            if (Bin->getOp() == BinaryOp::Sub) {
                if (isIV(Op1) && isRC(Op2, getOperandHeader(Op1))) {
                    IV = Op1;
                    RC = Op2;
                    return true;
                }
            }
        }
        return false;
    }
    Instruction *getOperandHeader(Value *Op) {
        if (auto *Var = Op->as<Instruction>()) {
            return mapSCCInfo[Var].header;
        }
        return nullptr;
    }

    // 将指令替换为强度削弱后的指令
    void doReplace(Instruction *Inst, Value *IV, Value *RC) {
        if (auto *Bin = Inst->as<BinaryInst>()) {
            auto *NewInst = doReduce(Bin->getOp(), IV, RC);
            Bin->replaceAllUsesWith(NewInst);
        }
    }

    // 记录强度削减后的表达式
    VNTable<Instruction *> ReduceTable;

    /**
     * 当遇到 t_0 = i_2 * 4 这样的候选操作我们可以进行强度削弱
     * 调用时分析出 i_2为归纳变量 4为区域常量
     */
    Instruction *doReduce(BinaryOp opcode, Value *IV, Value *RC) {
        auto entry = VNExpr{IV, RC, opcode};
        auto iter = ReduceTable.find(entry);
        if (iter != ReduceTable.end()) {
            return iter->second;
        }
        auto *New = IV->cast<Instruction>()->clone();
        auto *IVHeader = getOperandHeader(IV);
        mapSCCInfo[New].header = IVHeader;
        ReduceTable[entry] = New;
        for (auto &Use : New->operands()) {
            /**
             * i_2 = i_1 + 1
             * i_1 = phi (0, i_2)
             * 首先对归纳变量i_0所在的SCC递归的Clone出一个新的操作直到遇到Phi
             * 遇到i_1的时候会复制 i_1 = phi(0, i_2) 这个语句
             * 当遇到区域常量操作数我们应用doApply将归纳变量的递增大小按照RC扩大
             * 如遇到1的时候应用doApply, 将常数变为4, 如果遇到的不是常数则可以生成 x = y * rc指令进行扩大
             */
            auto *Op = Use.getValue();
            auto *OpHeader = getOperandHeader(Op);
            if (OpHeader == IVHeader) {
                Use.set(doReduce(opcode, Op, RC));
            } else if (opcode == BinaryOp::Mul || New->getOpcode() == OpcodePhi) {
                // 当复制的语句为Phi节点时, 而且当前的操作数不是SCC中的节点, 说明为初始值, 也需要调用doApply进行范围扩大
                // 当遇到的需要进行削减的是乘法指令并且操作数是另外一个归纳变量
                // a_0 = i_1 * 4  ==> IV = i_1,  RC = 4,  Opcode = Mul
                // i_1 = phi (0, i_2)
                // i_2 = i_1 + f_2
                // 如果此时f_2为归纳变量则需要递归调用doReduce使用RC进行扩大
                if (OpHeader) {
                    // 此时的Op属于另外一个归纳变量
                    Op = doReduce(opcode, Op, RC);
                } else {
                    Op = doApply(New, opcode, Op, RC);
                }
                Use.set(Op);
            }
        }
        return New;
    }

    Value *doApply(Instruction *Inst, BinaryOp opcode, Value *Left, Value *RC) {
        auto entry = VNExpr{Left, RC, opcode};
        auto iter = ReduceTable.find(entry);
        if (iter != ReduceTable.end()) {
            return iter->second;
        }
        ConstantFolder folder(opcode, Left, RC);
        if (auto *Res = folder.fold()) {
            return Res;
        }
        auto *Res = BinaryInst::Create(opcode, Left, RC);
        ReduceTable[entry] = Res;
        auto *Dom = Inst->getParent()->getDominator();
        assert(Dom);
        Dom->append(Res);
        return Res;
    }

    void runOnFunction(Function &function) override {
        function.forEach([&](Instruction *Inst) {
            DFS(Inst);
        });
        for (auto *Cur : worklist) {
            Value *IV, *RC;
            if (isCandidateOperation(Cur, IV, RC)) {
                doReplace(Cur, IV, RC);
            }
        }
        worklist.clear();
    }

};


#endif //DRAGON_OSR_H
