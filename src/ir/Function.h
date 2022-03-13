//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_FUNCTION_H
#define DRAGONIR_FUNCTION_H
#include <memory>
#include <Node.h>
#include <BasicBlock.h>
#include <SymbolTable.h>
#include <Type.h>
class Module;
class Type;
class Function : public Value, public NodeParent<Function, BasicBlock>, public NodeWithParent<Function, Module> {
public:
    static Function *Create(Module *module, std::string_view name, Type *type);
public:
    Function(Module *m, std::string_view name, Type *ft) : module(m), name(name), type(ft) {
        // FIXME: Module contains function?
    }

    const std::string &getName() const {
        return name;
    }

    Type *getType() override {
        return type;
    }

    Context *getContext() const;

    inline Type *getType() const {
        return type;
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

    BasicBlock *createBasicBlock(const std::string &bbName) {
        BasicBlock *BB = new BasicBlock(bbName);
        list.push_back(BB);
        return BB;
    }

    Param *addParam(std::string_view paramName, Type *paramType) {
        return params.emplace_back(new Param(paramName, paramType)).get();
    }

    template<typename InstTy, typename FnTy>
    void forEach(FnTy fn) {
        for (auto &BB: getSubList()) {
            for (auto &Inst: BB.getInstrs()) {
                if (auto *Val = Inst.as<InstTy>()) {
                    fn(Val);
                }
            }
        }
    }

    /// return true if the function is a declaration
    bool isDeclaration() const {
        return false;
    }

    void dump(std::ostream &os, int level = 0) override {
        os << "Function: " << getName() << " ";

        // dump params
        /*size_t I = 0;
        os << "(";
        for (;I < params.size(); ++I) {
            if(I > 0) os << ", ";
            type->getParameterType(I)->dump(os);
            params[I]->dumpAsOperand(os);
        }
        os << ") -> ";
        type->getReturnType()->dump(os);*/

        type->dump(os);

        os << std::endl;
        for (auto &BB: list) {
            BB.dump(os);
        }
    }
    void dumpAsOperand(std::ostream &os) override {
        os << "@" << getName();
    }

private:
    std::string name; /// function name
    Type *type = nullptr; /// function type

    Module *module = nullptr; /// the module that contains this function
    Function *outer = nullptr; /// the outer function

    std::vector<std::unique_ptr<Param>> params; /// function params
    SymbolTable<Instruction> symbolTable; /// symbol table for instructions

};


#endif //DRAGONIR_FUNCTION_H
