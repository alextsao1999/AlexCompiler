//
// Created by Alex on 2022/5/3.
//

#ifndef DRAGON_PATTERNDAG_H
#define DRAGON_PATTERNDAG_H

#include "PatternNode.h"
class PatternDAG {
public:
    NodeList<PatternNode> allNodes;
    PatternNode *rootNode = nullptr;

    void setRootNode(const std::vector<PatternNode *> &nodes) {
        rootNode = PatternNode::createNode<RootNode>(nodes);
    }

    void setRootNode(PatternNode *node) {
        rootNode = node;
        allNodes.push_back(node);
    }

    PatternNode *getRootNode() {
        return rootNode;
    }

};

#endif //DRAGON_PATTERNDAG_H
