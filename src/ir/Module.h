//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_MODULE_H
#define DRAGONIR_MODULE_H


#include <string>
#include <map>
#include <memory>

#include "Node.h"
#include "Function.h"

class Module : public NodeParent<Module, Function> {
public:
    explicit Module(std::string_view name) : name(name) {}

    Function *createFunction(std::string_view fn) {
        Function *F = new Function(fn);
        append(F);
        functions.emplace(fn, F);
        return F;
    }

    Function *getFunction(const std::string &fn) {
        return functions.at(fn);
    }

    const std::string &getName() const {
        return name;
    }

    auto begin() {
        return functions.begin();
    }

    auto end() {
        return functions.end();
    }

private:
    std::map<std::string, Function *> functions;

    std::string name;

};


#endif //DRAGONIR_MODULE_H
