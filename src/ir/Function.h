//
// Created by Alex on 2022/3/8.
//

#ifndef DRAGONIR_FUNCTION_H
#define DRAGONIR_FUNCTION_H
#include <memory>
#include <list>
#include <Node.h>
#include <BasicBlock.h>
#include <SymbolTable.h>
#include <Type.h>
#include <LoopInfo.h>
#include <MachineBlock.h>
#include <Target.h>
class Module;
class Type;
class Function : public Value, public NodeParent<Function, BasicBlock>, public NodeWithParent<Function, Module> {
public:
    static Function *Create(Module *module, StrView name, Type *type);
    static Function *Create(StrView name, Type *type);
public:
    Function(Module *parent, StrView name, Type *ft);
    Function(StrView name, Type *ft) : name(name), type(ft) {}

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

    inline Module *getModule() const {
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

    inline iterator begin() {
        return getSubList().begin();
    }

    inline iterator end() {
        return getSubList().end();
    }

    BasicBlock *createBasicBlock(const std::string &bbName) {
        BasicBlock *BB = new BasicBlock(bbName);
        list.push_back(BB);
        return BB;
    }

    Param *addParam(StrView paramName, Type *paramType) {
        auto *P = new Param(paramName, paramType);
        params.emplace_back(P);
        return P;
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
    }

    template<typename Fn>
    inline void forEach(Fn fn) {
        forEachBlock([&](BasicBlock *bb) {
            auto InstIter = bb->begin();
            auto InstEnd = bb->end();
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
        auto Iter = begin();
        auto End = end();
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

    TargetInfo *getTargetInfo() {
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
public:
    ///< machine blocks
    NodeList<MachineBlock> blocks;
    std::map<BasicBlock *, MachineBlock *> mapBlocks;
    std::map<PatternNode *, std::set<Operand *>> mapOperands;
    PatternDAG dag;
};

#endif //DRAGONIR_FUNCTION_H
