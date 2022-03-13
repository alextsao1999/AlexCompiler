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

    bool hasName(T *item) const {
        return nameTable.template count(item);
    }

    void setName(T *item, StrView name) {
        if (name.empty()) {
            nameTable.erase(item);
        } else {

            /*auto &Count = countTable[std::string(name)];
            nameTable[item] = std::string(name) + "." + std::to_string(Count++);*/

            nameTable[item] = name;
        }
    }

    const std::string &getName(T *item) {
        // allocate a temp name if not found
        auto It = nameTable.find(item);
        if (It == nameTable.end()) {
            nameTable[item] = std::to_string(count++);
            return nameTable[item];
        }
        return It->second;
    }

private:
    size_t count = 0;
    std::map<std::string, size_t> countTable;
    std::map<std::string, T *> valueTable;
    std::map<T *, std::string> nameTable;

};

#endif //DRAGONIR_SYMBOLTABLE_H
