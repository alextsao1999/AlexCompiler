//
// Created by Alex on 2022/3/19.
//

#ifndef DRAGON_GLOBALTABLE_H
#define DRAGON_GLOBALTABLE_H
#include <string>
#include "Type.h"

struct Scope {
    std::map<std::string, Type *> typeTable;
    std::map<std::string, Value *> varTable;
};

class GlobalTable {
    std::vector<Scope> scopes;
public:
    GlobalTable() {
        enterScope();
    }
    ~GlobalTable() {}

    void enterScope() {
        scopes.emplace_back();
    }

    void leaveScope() {
        scopes.pop_back();
    }

    void addType(const std::string &name, Type *type) {
        scopes.back().typeTable[name] = type;
    }

    Type *getType(const std::string &name) {
        for (auto It = scopes.rbegin(); It != scopes.rend(); ++It) {
            auto It2 = It->typeTable.find(name);
            if (It2 != It->typeTable.end()) {
                return It2->second;
            }
        }
        return nullptr;
    }

    void addVar(const std::string &name, Value *var) {
        scopes.back().varTable[name] = var;
    }

    Value *getVar(const std::string &name) {
        for (auto It = scopes.rbegin(); It != scopes.rend(); ++It) {
            auto It2 = It->varTable.find(name);
            if (It2 != It->varTable.end()) {
                return It2->second;
            }
        }
        return nullptr;
    }

    bool hasVar(const std::string &name) {
        for (auto It = scopes.rbegin(); It != scopes.rend(); ++It) {
            auto It2 = It->varTable.find(name);
            if (It2 != It->varTable.end()) {
                return true;
            }
        }
        return false;
    }

};


#endif //DRAGON_GLOBALTABLE_H
