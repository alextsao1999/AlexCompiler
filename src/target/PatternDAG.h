//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_PATTERNDAG_H
#define DRAGON_PATTERNDAG_H

#include <sstream>
#include "Constant.h"
#include "PatternNode.h"
#include "MachineBlock.h"
class PatternDAG {
public:
    NodeList<PatternNode> allNodes;
    PatternNode *rootNode = nullptr;

    void setRootNode(PatternNode *node) {
        rootNode = node;
        addNode(node);
    }

    PatternNode *getCopyToReg(PatternNode *reg, PatternNode *val) {
        CopyToReg *Copy = new CopyToReg(reg, val);
        addNode(Copy);
        return Copy;
    }
    PatternNode *getCopyFromReg(Value *val) {
        CopyFromReg *Copy = new CopyFromReg(val);
        addNode(Copy);
        return Copy;
    }

    PatternNode *getPhyReg(Register reg) {
        auto *Node = new PhyRegNode(reg);
        addNode(Node);
        return Node;
    }

    PatternNode *getVirReg(PatternNode *value) {
        auto *Node = new VirRegNode(value);
        addNode(Node);
        return Node;
    }

    int index = 0;
    void addNode(PatternNode *node) {
        node->index = index++;
        allNodes.push_back(node);
    }

    PatternNode *getRootNode() {
        return rootNode;
    }

    std::string dump() {
        std::stringstream ss;
        ss << "graph TD;" << std::endl;
        for (auto &node: allNodes) {
            ss << "    ";
            switch ((Pattern::Opcode) node.getOpcode()) {
                case Pattern::PhyRegister:
                    ss << node.index << "(PhyReg: " << ((PhyRegNode *) &node)->getRegister() << ")" << std::endl;
                    break;
                case Pattern::VirRegister:
                    ss << node.index << "(VirReg)" << std::endl;
                    break;
                case Pattern::Constant:
                    ss << node.index << "(Const: ";
                    if (auto *Val = ((ConstantNode *) &node)->getValue()) {
                        if (Val->as<IntConstant>()) {
                            ss << Val->as<IntConstant>()->getVal();
                        }
                    }
                    ss << ")" << std::endl;
                    break;
                case Pattern::BlockAddress:
                    ss << node.index << "(block)" << std::endl;
                    break;
                case Pattern::Address:
                    ss << node.index << "(addr)" << std::endl;
                    break;
                case Pattern::Root:
                    ss << node.index << "(block: ";
                    if (auto *Val = ((RootNode *) &node)->getBlock()) {
                        ss << Val->getName();
                    }
                    ss << ")" << std::endl;
                    break;
                default:
                    ss << node.index << "(" << Pattern::dump(node.getOpcode()) << ")" << std::endl;
                    break;
            }

            static std::map<unsigned, std::vector<std::string>> edges = {
                    {Pattern::CopyToReg, {"To", "From"}},
                    {Pattern::CondJump, {"Cond", "True", "False"}},
            };
            auto GetEdge = [&](int i) {
                if (edges.count(node.getOpcode())) {
                    return edges[node.getOpcode()][i];
                }
                return "\"[" + std::to_string(i + 1) + "]\"";
            };
            for (int i = 0; i < node.getNumOperands(); ++i) {
                ss << "        ";
                if (auto *Child = node.getChild(i)) {
                    ss << node.index << " -- " << GetEdge(i) << " --> " << Child->index << std::endl;
                } else {
                    ss << node.index << " -- " << GetEdge(i) << " --> " << (index++) << "(null)" << std::endl;
                }
            }
        }
        return ss.str();
    }

};

#endif //DRAGON_PATTERNDAG_H
