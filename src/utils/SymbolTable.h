//
// Created by Alex on 2022/3/9.
//

#ifndef DRAGONIR_SYMBOLTABLE_H
#define DRAGONIR_SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <map>
template<typename T>
class SymbolTable {
public:

    bool hasName(T *Item) const {
        return NameTable.template count(Item);
    }

    void setName(T *Item, std::string_view Name) {
        NameTable[Item] = Name;
    }

    const std::string &getName(T *Item) {
        // allocate a temp name if not found
        auto it = NameTable.find(Item);
        if (it == NameTable.end()) {
            NameTable[Item] = std::to_string(Count++);
            return NameTable[Item];
        }
        return it->second;
    }

private:
    size_t Count = 0;
    std::map<std::string, T *> ValueTable;
    std::map<T *, std::string> NameTable;

};

#endif //DRAGONIR_SYMBOLTABLE_H
