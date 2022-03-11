//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_FUNCTION_H
#define DRAGONIR_FUNCTION_H

#include <Node.h>
#include <BasicBlock.h>
#include <SymbolTable.h>
class Module;
class Type;
class Function : public NodeParent<Function, BasicBlock>, public NodeWithParent<Function, Module>, public Value {
public:
    static Function *Create(Module *module, std::string_view name, Type *type);
public:
    Function(Module *m, std::string_view name, Type *ft) : module(m), name(name), type(ft) {

    }

    const std::string &getName() const {
        return name;
    }

    Type *getType() override {
        return type;
    }

    inline Context *getContext() const;

    inline Type *getType() const {
        return type;
    }

    BasicBlock *getEntryBlock() {
        assert(!list.empty());
        return list.begin().getPointer();
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
    Type *type = nullptr;

    Module *module = nullptr;
    Function *outer = nullptr; // 函数的外部函数


    SymbolTable<Instruction> symbolTable;

};


#endif //DRAGONIR_FUNCTION_H
