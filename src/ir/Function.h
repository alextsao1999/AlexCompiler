﻿//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_FUNCTION_H
#define DRAGONIR_FUNCTION_H
#include <memory>
#include <Node.h>
#include <BasicBlock.h>
#include <SymbolTable.h>
#include <Type.h>
#include <LoopInfo.h>
class Module;
class Type;
class Function : public Value, public NodeParent<Function, BasicBlock>, public NodeWithParent<Function, Module> {
public:
    static Function *Create(Module *module, std::string_view name, Type *type);
    static Function *Create(std::string_view name, Type *type);
public:
    Function(Module *parent, std::string_view name, Type *ft);
    Function(std::string_view name, Type *ft) : name(name), type(ft) {}

    const std::string &getName() const {
        return name;
    }

    inline Type *getType() override {
        return type;
    }

    inline Type *getReturnType() const {
        return type->getReturnType();
    }

    inline Type *getType() const {
        return type;
    }

    Module *getModule() const {
        return module;
    }

    Context *getContext() const {
        assert(getType());
        return getType()->getContext();
    }

    BasicBlock *getEntryBlock() const {
        assert(!list.empty());
        return list.begin().getPointer();
    }

    auto &getBasicBlockList() {
        return list;
    }

    auto &getSymbolTable() {
        return symbolTable;
    }

    auto &getSymbolTable() const {
        return symbolTable;
    }

    inline auto begin() {
        return getSubList().begin();
    }

    inline auto end() {
        return getSubList().end();
    }

    BasicBlock *createBasicBlock(const std::string &bbName) {
        BasicBlock *BB = new BasicBlock(bbName);
        list.push_back(BB);
        return BB;
    }

    Param *addParam(std::string_view paramName, Type *paramType) {
        return params.emplace_back(new Param(paramName, paramType)).get();
    }

    Param *getParam(unsigned index) {
        assert(index < params.size());
        return params[index].get();
    }

    void eraseParam(unsigned index) {
        assert(index < params.size());
        params.erase(params.begin() + index);
    }

    auto &getParams() {
        return params;
    }

    template<typename InstTy, typename FnTy>
    inline void forEach(FnTy fn) {
        forEach([&](Instruction *inst) {
            if (auto *I = inst->template as<InstTy>()) {
                fn(I);
            }
        });
        /*for (auto &BB: getSubList()) {
            for (auto &Inst: BB.getInstrs()) {
                if (auto *Val = Inst.as<InstTy>()) {
                    fn(Val);
                }
            }
        }*/
    }

    template<typename Fn>
    inline void forEach(Fn fn) {
        forEachBlock([&](BasicBlock *bb) {
            auto InstIter = bb->getSubList().begin();
            auto InstEnd = bb->getSubList().end();
            if (InstIter != InstEnd) {
                do {
                    auto *Inst = &(*InstIter++);
                    fn(Inst);
                } while (InstIter != InstEnd);
            }
        });
    }

    template<typename Fn>
    inline void forEachBlock(Fn fn) {
        auto Iter = list.begin();
        auto End = list.end();
        if (Iter != End) {
            do {
                auto *BB = &(*Iter++);
                fn(BB);
            } while (Iter != End);
        }
    }

    ///< return true if the function is a declaration
    bool isDeclaration() const {
        return false;
    }

    ///< call graph
    void addCallee(Function *caller) {
        callees.insert(caller);
        caller->callers.insert(this);
    }

    ///< dump the function
    void dump(std::ostream &os) override {
        os << "def " << getName();

        // dump params
        size_t I = 0;
        os << "(";
        for (;I < params.size(); ++I) {
            if(I > 0) os << ", ";
            params[I]->dumpAsOperand(os);
        }
        os << ") -> ";
        type->getReturnType()->dump(os);

        //type->dump(os);

        os << " {" << std::endl;
        /*DUMP_REF_S(os, list, "\n", BB, {
            BB.dump(os);
        });*/
        os << dump_str(list, ValueDumper(), "\n\n");
        os << std::endl << "}" << std::endl;
    }
    void dumpAsOperand(std::ostream &os) override {
        assert(getReturnType());
        getReturnType()->dump(os);
        os << " @" << getName();
    }

    BasicBlock *getBlockByName(const std::string &bbname) {
        for (auto &BB: list) {
            if (BB.getName() == bbname) {
                return &BB;
            }
        }
        return nullptr;
    }

    ///< function loops
    std::list<Loop> loops;
private:
    ///< function name
    std::string name;
    ///< function type
    Type *type = nullptr;
    ///< the module that contains this function
    Module *module = nullptr;
    ///< the outer function
    Function *outer = nullptr;
    ///< function params
    std::vector<std::unique_ptr<Param>> params;
    ///< symbol table for instructions
    SymbolTable symbolTable;

    ///< CallGraph
    std::set<Function *> callers;
    std::set<Function *> callees;
};

#endif //DRAGONIR_FUNCTION_H
