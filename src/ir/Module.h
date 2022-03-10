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
class Context;
class Module : public NodeParent<Module, Function> {
public:
    explicit Module(std::string_view name, Context &context) : name(name), context(&context) {}

    inline Function *createFunction(std::string_view fn) {
        Function *F = new Function(this, fn);
        append(F);
        functions.emplace(fn, F);
        return F;
    }

    const std::string &getName() const {
        return name;
    }

    Function *getFunction(const std::string &fn) {
        return functions.at(fn);
    }

    Context *getContext() const {
        return context;
    }

    inline auto begin() {
        return getSubList().begin();
    }

    inline auto end() {
        return getSubList().end();
    }

private:
    Context *context = nullptr;
    std::map<std::string, Function *> functions;
    std::map<std::string, Value *> globals;

    std::string name;

};


#endif //DRAGONIR_MODULE_H
