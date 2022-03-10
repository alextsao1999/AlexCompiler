//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_FUNCTION_H
#define DRAGONIR_FUNCTION_H

#include <Node.h>
#include <BasicBlock.h>
#include <SymbolTable.h>
class Module;
class Function : public NodeParent<Function, BasicBlock>, public NodeWithParent<Function, Module>, public Value {
public:
    static Function *Create(Module *module, std::string_view name);
public:
    Function(Module *m, std::string_view name) : module(m), name(name) {}

    const std::string &getName() const {
        return name;
    }

    inline Context *getContext() const;

    inline Type *getType() const {
        return type;
    }

    auto &getBasicBlockList() {
        return list;
    }

    auto &getSymbolTable() {
        return symbolTable;
    }

    BasicBlock *createBasicBlock(const std::string &bbName) {
        BasicBlock *BB = new BasicBlock(bbName);
        list.push_back(BB);
        return BB;
    }

    void dump(std::ostream &os, int level = 0) override {
        os << "Function: " << getName() << std::endl;
        for (auto &BB: list) {
            BB.dump(os);
        }
    }

    void dumpAsOperand(std::ostream &os) override {
        os << "@" << getName();
    }

private:
    std::string name;
    SymbolTable<Instruction> symbolTable;
    Module *module = nullptr;
    Type *type = nullptr;

};


#endif //DRAGONIR_FUNCTION_H
