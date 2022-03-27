//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "Module.h"

Function *Function::Create(Module *module, std::string_view name, Type *type) {
    assert(module);
    return module->createFunction(name, type);
}

Function::Function(Module *parent, std::string_view name, Type *ft) : module(parent), name(name), type(ft) {
    // FIXME: Module contains function?
}

Function *Function::Create(std::string_view name, Type *type) {
    return new Function(name, type);
}
