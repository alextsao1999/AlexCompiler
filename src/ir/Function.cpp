//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "Module.h"

Function *Function::Create(Module *module, StrView name, Type *type) {
    ASSERT(module);
    return module->createFunction(name, type);
}

Function::Function(Module *parent, StrView name, Type *ft) : module(parent), name(name), type(ft) {
    // FIXME: Module contains function?
}

Function *Function::Create(StrView name, Type *type) {
    return new Function(name, type);
}
