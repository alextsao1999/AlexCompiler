//
// Created by Alex on 2022/3/9.
//

#ifndef DRAGONIR_SYMBOLTABLE_H
#define DRAGONIR_SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <map>

class Value;
class SymbolTable;

class SymbolTable {
public:
    bool hasName(Value *item) const {
        return nameTable.count(item);
    }
    std::string &getName(Value *item) {
        // allocate a temp name if not found
        auto It = nameTable.find(item);
        if (It == nameTable.end()) {
            auto &Str = nameTable[item] = "";
            //addCount(item, Str);
            return Str;
        }
        return It->second;
    }
    void setName(Value *item, StrView name) {
        removeName(item);
        nameTable[item] = name;
        //addCount(item, std::string(name));
    }
    void removeName(Value *item) {
        if (!nameTable.count(item)) {
            return;
        }
        auto &Name = nameTable[item];
        nameTable.erase(item);
        removeCount(item, Name);
    }
    void removeCount(Value *item, const std::string &name) {
        auto &List = countTable[name];
        auto Iter = std::find(List.begin(), List.end(), item);
        if (Iter != List.end()) {
            List.erase(Iter);
        }
    }
    void addCount(Value *item, const std::string &name = "") {
        countTable[name].push_back(item);
    }

    unsigned getCount(Value *item) {
        return getCount(item, getName(item));
    }
    unsigned getCount(Value *item, const std::string &name) {
        auto &List = countTable[name];
        auto Iter = std::find(List.begin(), List.end(), item);
        if (Iter == List.end()) {
            Iter = List.insert(List.end(), item);
        }
        return Iter - List.begin();
    }
    unsigned getNameSize(const std::string &name) {
        auto It = countTable.find(name);
        if (It == countTable.end()) {
            return 0;
        }
        return It->second.size();
    }

private:
    std::unordered_map<Value *, std::string> nameTable;
    std::unordered_map<std::string, std::vector<Value *>> countTable;
};

#endif //DRAGONIR_SYMBOLTABLE_H
