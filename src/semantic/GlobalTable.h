//
// Created by Alex on 2022/3/19.
//

#ifndef DRAGON_GLOBALTABLE_H
#define DRAGON_GLOBALTABLE_H
#include <string>
#include "Type.h"

struct Scope {
    std::map<std::string, Type *> symbolTable;
};

class GlobalTable {
    std::vector<Scope> scopes;
public:
    GlobalTable() {}
    ~GlobalTable() {}

    void enterScope() {
        scopes.emplace_back();
    }

    void exitScope() {
        scopes.pop_back();
    }

    void add(const std::string &name, Type *type) {
        scopes.back().symbolTable[name] = type;
    }

    Type *getType(const std::string &name) {
        for (auto It = scopes.rbegin(); It != scopes.rend(); ++It) {
            auto It2 = It->symbolTable.find(name);
            if (It2 != It->symbolTable.end()) {
                return It2->second;
            }
        }
        return nullptr;
    }

};


#endif //DRAGON_GLOBALTABLE_H
