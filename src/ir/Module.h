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
#include "RISCVTarget.h"

class Context;
class Module : public NodeParent<Module, Function> {
    RISCVTarget target;
public:
    explicit Module(StrView name, Context &context) : name(name), context(&context) {}

    inline Function *createFunction(StrView fn, Type *type) {
        Function *F = new Function(this, fn, type);
        F->setTargetInfo(&target);
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

    Function *getFunction(size_t index = 0) {
        for (auto Iter = functions.begin(); Iter != functions.end(); ++Iter) {
            if (index-- == 0) {
                return Iter->second;
            }
        }
        return nullptr;
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

    std::string dumpToString() {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }

private:
    std::string name;

    Context *context = nullptr;
public:
    std::map<std::string, Function *> functions;
    std::map<std::string, Value *> globals;

};

#endif //DRAGONIR_MODULE_H
