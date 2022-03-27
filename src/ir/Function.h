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
    void forEach(FnTy fn) {
        auto BBIt = list.begin();
        auto BBEnd = list.end();
        if (BBIt == BBEnd) {
            return;
        }
        do {
            auto &BB = *BBIt++;
            auto InstIter = BB.getSubList().begin();
            auto InstEnd = BB.getSubList().end();

            if (InstIter == InstEnd) {
                continue;
            }
            do {
                auto &Inst = *InstIter++;
                if (auto *I = Inst.template as<InstTy>()) {
                    fn(I);
                }
            } while (InstIter != InstEnd);
        } while (BBIt != BBEnd);

        /*for (auto &BB: getSubList()) {
            for (auto &Inst: BB.getInstrs()) {
                if (auto *Val = Inst.as<InstTy>()) {
                    fn(Val);
                }
            }
        }*/
    }

    template<typename Fn>
    void forEach(Fn fn) {
        auto BBIt = list.begin();
        auto BBEnd = list.end();
        if (BBIt == BBEnd) {
            return;
        }
        do {
            auto &BB = *BBIt++;
            auto InstIter = BB.getSubList().begin();
            auto InstEnd = BB.getSubList().end();
            if (InstIter == InstEnd) {
                continue;
            }
            do {
                auto &Inst = *InstIter++;
                fn(&Inst);
            } while (InstIter != InstEnd);
        } while (BBIt != BBEnd);
    }

    /// return true if the function is a declaration
    bool isDeclaration() const {
        return false;
    }

    /// call graph
    void addCallee(Function *caller) {
        callees.insert(caller);
        caller->callers.insert(this);
    }

    /// dump the function
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
        DUMP_REF_S(os, list, "\n", BB, {
            BB.dump(os);
        });
        os << "}" << std::endl;
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

    std::list<Loop> loops;
private:
    std::string name; /// function name
    Type *type = nullptr; /// function type

    Module *module = nullptr; /// the module that contains this function
    Function *outer = nullptr; /// the outer function

    std::vector<std::unique_ptr<Param>> params; /// function params
    SymbolTable<Instruction> symbolTable; /// symbol table for instructions

    // CallGraph
    std::set<Function *> callers;
    std::set<Function *> callees;

};


#endif //DRAGONIR_FUNCTION_H
