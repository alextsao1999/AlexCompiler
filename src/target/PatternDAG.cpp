//
// Created by Alex on 2022/5/3.
//

#include "PatternDAG.h"
#include "Function.h"
std::string PatternDAG::dump() {
    std::stringstream ss;
    ss << "graph TD;" << std::endl;
    for (auto &node: allNodes) {
        ss << "    ";
        switch ((Pattern::Opcode) node.getOpcode()) {
            case Pattern::Register:
                if (auto *RegN = node.as<RegisterNode>()) {
                    auto Reg = RegN->getReg();
                    ss << node.index;
                    if (Reg.isVirReg()) {
                        ss << "(VReg: ";
                    } else {
                        ss << "(PReg: ";
                    }
                    ss << Reg.getRegId() << ")" << std::endl;
                }
                continue;
            case Pattern::Constant:
                ss << node.index << "(Const: ";
                if (auto *Val = node.as<ConstantNode>()->getValue()) {
                    if (Val->as<IntConstant>()) {
                        ss << Val->as<IntConstant>()->getVal();
                    }
                }
                ss << ")" << std::endl;
                break;
            case Pattern::BlockAddress:
                if (auto *Block = node.as<BlockAddress>()) {
                    ss << node.index << "(addr: ";
                    Block->getValue()->dumpAsOperand(ss);
                    ss << ")" << std::endl;

                    /*auto *BB = Block->getValue();
                    auto *B = BB->getParent()->mapBlocks[BB];
                    ss << "        " << node.index << "-->" << B->getRootNode()->index << std::endl;*/
                }
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
