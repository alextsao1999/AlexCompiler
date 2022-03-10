//
// Created by Alex on 2022/3/8.
//

#include "Function.h"
#include "Module.h"
Function *Function::Create(Module *module, std::string_view name) {
    return module->createFunction(name);
}
