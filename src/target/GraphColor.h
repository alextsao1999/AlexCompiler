//
// Created by Alex on 2022/5/4.
//

#ifndef DRAGON_GRAPHCOLOR_H
#define DRAGON_GRAPHCOLOR_H
#include <set>
#include <queue>
#include <unordered_set>
#include <cmath>

#include "MachinePass.h"
#include "PatternNode.h"

using ColorTy = unsigned;

class GraphNode {
public:
    std::set<GraphNode *> Edges; // Solid edges
    std::set<GraphNode *> Sames; // Dash edges (move operation)
    Register Reg;
    int Degree = 0;
    unsigned SpillSlot = 0;
    bool isSpilled = false;
    bool isTemp = true;
    void addEdge(GraphNode *node) {
        if (node == this) {
            return;
        }
        Edges.insert(node);
        node->Edges.insert(this);
    }
    void addSame(GraphNode *node) {
        if (node == this) {
            return;
        }
        Sames.insert(node);
        node->Sames.insert(this);
    }
};

struct GraphCompare {
    inline bool operator()(const GraphNode *LHS, const GraphNode *RHS) const {
        return LHS->Degree < RHS->Degree;
    }
};
using OpStack = std::priority_queue<GraphNode *, std::vector<GraphNode *>, GraphCompare>;

struct ColorHasher {
    using Node = std::pair<GraphNode *, ColorTy>;
    inline size_t operator()(const Node &node) const {
        return reinterpret_cast<size_t>(node.first) + node.second;
    }
};

class GraphColor : public MachinePass {
public:
    int RegisterCount = 0;
    Function *func = nullptr;
    std::unordered_map<RegID, GraphNode> graph;
    std::vector<GraphNode *> stack;
    std::unordered_set<std::pair<GraphNode *, ColorTy>, ColorHasher> usedColor;
    std::unordered_map<GraphNode *, ColorTy> nodeColor; //< Allocated register
    std::unordered_map<GraphNode *, double> spillCosts; //< Spill costs

    void buildIGraph() {
        graph.clear();
        auto *TI = func->getTargetInfo();
        ASSERT(TI);
        for (auto &MBB: func->blocks) {
            auto &Live = MBB.liveOutSet;
            for (auto &Inst: MBB.instrs().reverse()) {
                TargetOpcode opcode = (TargetOpcode)Inst.getOpcode();
                for (auto &Def: Inst.defs()) {
                    if (TI->isMove(Inst)) {
                        for (auto &Op: Inst.uses()) {
                            if (Op.isReg()) {
                                // FIXME: I don't know whether it is correct.
                                // But it works for one op move instruction.
                                graph[Def.getReg()].addSame(&graph[Op.getReg()]);
                            }
                        }
                    }
                    for (auto &Item: Live) {
                        graph[Item].addEdge(&graph[Def.getReg()]);
                    }
                    Live.erase(Def.getReg());
                }

                if (func->getTargetInfo()->isCall(Inst)) {
                    for (auto &Alive: Live) {
                        graph[Alive].isTemp = false;
                    }
                }

                for (auto &Use: Inst.uses()) {
                    if (Use.isReg()) {
                        Live.insert(Use.getReg());
                    }
                }
            }
        }
        for (auto &[op, node]: graph) {
            node.Reg = op;
        }
    }

    void simplify() {
        stack.clear();
        std::vector<GraphNode *> Nodes;
        // 计算所有的度
        for (auto &[op, node]: graph) {
            if (node.isSpilled) {
                continue;
            }
            if (Register::isPhyReg(op)) {
                nodeColor[&node] = op;
                continue;
            }
            node.Degree = 0;
            for (auto &Edge: node.Edges) {
                if (!Edge->isSpilled)
                    node.Degree++;
            }
            Nodes.push_back(&node);
        }
        // 构建栈
        while (!Nodes.empty()) {
            // 寻找度最小的元素, 或者是寻找度小于k的节点
            auto Iter = std::min_element(Nodes.begin(), Nodes.end(),
                                         [](const GraphNode *lhs, const GraphNode *rhs) -> bool { return lhs->Degree < rhs->Degree; });
            auto *Node = *Iter;
            Nodes.erase(Iter);
            Node->Degree = 0;
            for (auto *Edge: Node->Edges) {
                if (std::find(Nodes.begin(), Nodes.end(), Edge) != Nodes.end()) {
                    Edge->Degree--;
                }
            }
            stack.push_back(Node);
        }
    }

    bool coalesce(GraphNode *node) {
        // Coalescing consists of collapsing two move related nodes together.
        // Briggs: Nodes a and b can be coalesced if the resulting node ab will have fewer than K neighbors of high degree (i.e., degree ≥ K edges)
        // George: Nodes a and b can be coalesced if, for every neighbor t of a, either t already interferes with b, or t is of low degree
        return false;
        for (auto &Same: node->Sames) {
            if (node->Edges.size() == 1 || Same->Edges.size() == 1 ||
                node->Edges.size() == 0 || Same->Edges.size() == 0) {
                if (nodeColor.count(Same)) {
                    nodeColor[node] = nodeColor[Same];
                    return true;
                }
            }
        }
        return false;
    }

    void freeze() {
        // If neither simplify nor coalesce applies, we look for a move related edge, and remove it from the graph.
        // We are giving up the opportunity of coalescing these nodes.
    }

    void computeSpillCost() {
        /**
         * SPILLCOST(v) = (Σ(SB×10^N))/D, where
         *  SB is the number of uses and defs at B
         *   N is B's loop nesting factor
         *   D is v's degree in the interference graph
         */
        std::unordered_map<RegID, int> OpCount;
        spillCosts.clear();
        for (auto &MBB: func->blocks) {
            //std::cout << MBB.name << " level:" << MBB.level << std::endl;
            auto Scale = pow(10, MBB.level);
            OpCount.clear();
            for (auto &Inst: MBB.instrs()) {
                for (auto &Op: Inst.uses()) {
                    OpCount[Op.getReg()]++;
                }
                for (auto &Def: Inst.defs()) {
                    OpCount[Def.getReg()]++;
                }
            }
            for (auto [node, b]: OpCount) {
                spillCosts[&graph[node]] += Scale * b;
            }
        }
        for (auto &[node, sc]: spillCosts) {
            sc = sc / (node->Edges.size());
        }
    }

    bool tryGetColor(GraphNode *node, ColorTy &color) {
        auto iter = nodeColor.find(node);
        if (iter == nodeColor.end()) {
            return false;
        }
        color = iter->second;
        return true;
    }

    bool tryColorize(GraphNode *node) {
        if (coalesce(node)) {
            return true;
        }
        if (node->isTemp) {
            for (auto Reg: func->getTargetInfo()->getTempRegList()) {
                if (!usedColor.count({node, Reg})) {
                    // 找到周围没有的颜色, 进行染色
                    nodeColor[node] = Reg;
                    return true;
                }
            }
        } else {
            for (auto Reg: func->getTargetInfo()->getSaveRegList()) {
                if (!usedColor.count({node, Reg})) {
                    // 找到周围没有的颜色, 进行染色
                    nodeColor[node] = Reg;
                    return true;
                }
            }
        }

        return false;
    }

    void trySpill() {
        // 计算溢出开销
        //computeSpillCost();
        // 选择溢出开销最小的节点进行溢出
        /*auto iter = *std::min_element(stack.begin(), stack.end(), [&](GraphNode *LHS, GraphNode *RHS) -> bool {
            return spillCosts[LHS] < spillCosts[LHS];
        });

        iter->isSpilled = true;*/

        if (!stack.empty()) {
            stack.back()->isSpilled = true;
        }
    }

    bool select() {
        usedColor.clear();
        while (!stack.empty()) {
            auto *node = stack.back();
            if (nodeColor.count(node)) {
                // 节点已经着色, 不需要再进行着色了
                continue;
            }
            // 将所有分配颜色的边加入到集合中, 方便查找之前周围是否已染上某颜色
            for (auto *edge: node->Edges) {
                ColorTy color;
                if (tryGetColor(edge, color)) {
                    usedColor.insert({node, color});
                }
            }
            // 尝试染色, 无法染色就spill
            if (!tryColorize(node)) {
                trySpill();
                return true; // 继续分配过程
            }
            stack.pop_back();
        }
        return false;
    }

    void assignReg(Register virReg, Register phyReg) {
        for (auto &Op: func->mapOperands[virReg]) {
            Op->regOp = phyReg;
        }
        func->allocatedRegs.insert(phyReg);
    }

    void spillReg(GraphNode *spillNode, Register virReg) {
        int slot = 0;
        for (auto &Edge: spillNode->Edges) {
            if (Edge->isSpilled) {
                usedColor.insert({Edge, Edge->SpillSlot});
            }
        }
        while (usedColor.count({spillNode, slot})) {
            slot++;
        }
        spillNode->SpillSlot = slot;
        for (auto &Op: func->mapOperands[virReg]) {
            *Op = Operand::slot(slot);
        }
        func->spillSlotCount = func->spillSlotCount > slot ? func->spillSlotCount : slot;
        func->spillSlots[virReg] = slot;
    }

    void runOnFunction(Function &function) override {
        func = &function;
        auto *TI = func->getTargetInfo();
        RegisterCount = TI->getSaveRegList().size() + TI->getTempRegList().size();

        /*func->dumpMBB(std::cout);
        std::cout << "-------------------------------------------------------" << std::endl;*/
        buildIGraph();

        bool next;
        do {
            nodeColor.clear();
            usedColor.clear();
            simplify();
            next = select();
        } while (next);

        for (auto &[op, node]: graph) {
            Register Reg = op;
            if (node.isSpilled) {
                //< 将虚拟寄存器溢出
                spillReg(&node, Reg);
                continue;
            }
            ASSERT(nodeColor.count(&node));
            auto Color = nodeColor[&node];
            if (Reg.isVirReg()) {
                //< 寄存器指派
                assignReg(Reg, Color);
            }
        }
    }
};

#endif //DRAGON_GRAPHCOLOR_H
