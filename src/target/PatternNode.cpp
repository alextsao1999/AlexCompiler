//
// Created by Alex on 2022/3/13.
//

#include <new>
#include "PatternNode.h"

PatternNode *PatternNode::allocateWithUses(size_t size, const std::vector<PatternNode *> &nodes) {
    auto *Malloc = static_cast<char *>(::operator new(size + sizeof(PatternUse) * nodes.size()));
    PatternUse *Start = reinterpret_cast<PatternUse *>(Malloc);
    PatternUse *End = Start + nodes.size();
    PatternNode *Obj = reinterpret_cast<PatternNode *>(End);
    for (int I = 0; I < nodes.size(); ++I) {
        new(&Start[I]) PatternUse(Obj, nodes[I]);
    }
    return Obj;
}

void *PatternNode::operator new(size_t size, size_t useSize) {
    auto *Malloc = static_cast<char *>(::operator new(size + sizeof(PatternUse) * useSize));
    PatternUse *Start = reinterpret_cast<PatternUse *>(Malloc);
    PatternUse *End = Start + useSize;
    PatternNode *Obj = reinterpret_cast<PatternNode *>(End);
    for (; Start != End; ++Start)
        new(Start) PatternUse(Obj);
    return End;
}

void PatternNode::operator delete(void *p) {
    auto *Node = reinterpret_cast<PatternNode *>(p);
    auto *Ptr = reinterpret_cast<PatternUse *>(Node) - Node->getNumOperands();
    for (int I = 0; I < Node->getNumOperands(); ++I) {
        Ptr[I].~PatternUse();
    }
    ::operator delete(Ptr);
}

