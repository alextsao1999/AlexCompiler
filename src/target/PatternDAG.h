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

    PatternNode *getReg(Register reg) {
        auto *Node = new RegisterNode(reg);
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

    std::string dump();

};

#endif //DRAGON_PATTERNDAG_H
