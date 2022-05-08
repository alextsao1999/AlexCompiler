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
    explicit Module(StrView name, Context &context) : name(name), context(&context) {}

    inline Function *createFunction(StrView fn, Type *type) {
        Function *F = new Function(this, fn, type);
        append(F);
        functions.emplace(fn, F);
        return F;
    }

    const std::string &getName() const {
        return name;
    }

    Function *getFunction(const std::string &fn) {
        return functions[fn];
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

    void dump(std::ostream &os) {
        os << "Module: " << name;
        for (auto &F : *this) {
            os << std::endl;
            F.dump(os);
        }
    }

private:
    std::string name;

    Context *context = nullptr;
public:
    std::map<std::string, Function *> functions;
    std::map<std::string, Value *> globals;

};

#endif //DRAGONIR_MODULE_H
