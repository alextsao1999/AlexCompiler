//
// Created by Alex on 2022/3/19.
//

#ifndef DRAGON_CODEGEN_H
#define DRAGON_CODEGEN_H

#include "GlobalTable.h"
#include "Module.h"
#include "Context.h"
#include "Function.h"
#include "IRBuilder.h"
#include "parser.h"

class Diagnostic {
public:
    enum Level {
        Error,
        Warning,
        Fatal,
    };
private:
    std::vector<std::pair<Level, std::string>> diags;
public:
    void add(Level level, const std::string &msg) {
        diags.emplace_back(level, msg);
    }

    void add(Level level, const std::string &msg, value_t &value) {
        diags.emplace_back(level, msg);
    }

    void addWarning(const std::string &msg, value_t &value) {
        add(Level::Warning, msg, value);
    }

    void addError(const std::string &msg, value_t &value) {
        add(Level::Error, msg, value);
    }

    void addFatal(const std::string &msg, value_t &value) {
        add(Level::Fatal, msg, value);
    }

    void addWarning(const std::string &msg) {
        add(Level::Warning, msg);
    }

    void addError(const std::string &msg) {
        add(Level::Error, msg);
    }

    void addFatal(const std::string &msg) {
        add(Level::Fatal, msg);
    }

    void print(std::ostream &os) const {
        for (auto &Diag : diags) {
            switch (Diag.first) {
                case Level::Error:
                    os << "error: ";
                    break;
                case Level::Warning:
                    os << "warning: ";
                    break;
                case Level::Fatal:
                    os << "fatal: ";
                    break;
            }
            os << Diag.second << std::endl;
        }
    }
};

/**
 * The code generator for SysY.
 * @Author Alex
 */
class Codegen : public Visitor<Codegen, Value *> {
    ///< Compiler Context
    Context &context;
    ///< The global table.
    GlobalTable table;
    ///< The current module for compile unit.
    std::unique_ptr<Module> curModule;
    ///< The current function for compile unit.
    Function *curFunc = nullptr;
    ///< The current type for the var def.
    Type *curDefType = nullptr;
    ///< The IRBuilder for codegen.
    IRBuilder builder;
    ///< The map records all the undef functions.
    std::unordered_map<std::string, std::vector<CallInst *>> undefs;
    ///< The diagnostics data.
    Diagnostic diags;
public:
    Codegen(Context &context) : context(context) {
        table.addType("void", context.getVoidTy());
        table.addType("int", context.getInt32Ty());
        table.addType("float", context.getFloatTy());
    }

    const Diagnostic &getDiags() const {
        return diags;
    }

    std::unique_ptr<Module> &getModule() {
        return curModule;
    }

    Value *visitCompUnit(CompUnit value) override {
        curModule = std::make_unique<Module>("Module", context);
        return visit(value.getValue());
    }

    Value *visitFuncDef(FuncDef value) override {
        table.enterScope();
        // check redefine
        if (table.getVar(value.getName())) {
            diags.addError("redefine function name: " + value.getName(), value);
        }
        auto *RetTy = table.getType(value.getType());
        if (!RetTy) {
            diags.addError("undefined type: " + value.getType(), value);
            return nullptr;
        }
        auto *FuncTy = context.getFunctionTy(RetTy);
        curFunc = Function::Create(curModule.get(), value.getName(), FuncTy);
        // codegen params
        visit(value.getParams());
        // generate entry block
        auto *BB = BasicBlock::Create(curFunc, "entry");
        builder.setInsertPoint(BB);
        // codegen body
        visit(value.getBody());
        table.leaveScope();

        if (auto *Last = builder.getInsertBlock()) {
            if (!Last->getTerminator()) {
                builder.createRet();
            }
        }
        return nullptr;
    }

    Value *visitFuncParam(FuncParam value) override {
        assert(curFunc);
        auto *Param = curFunc->addParam(value.getName(), table.getType(value.getType()));
        table.addVar(value.getName(), Param);
        return Param;
    }

    Value *visitVarDecl(VarDecl value) override {
        curDefType = table.getType(value.getType());
        visit(value.getDefs());
        return nullptr;
    }

    Value *visitVarDef(VarDef value) override {
        if (table.hasVar(value.getName())) {
            // FIXME: Support shadowing?
            diags.addError("redefine variable name: " + value.getName(), value);
            return nullptr;
        }
        assert(curDefType);
        if (curDefType->isVoidType()) {
            diags.addError("void type can't be used as variable", value);
            return nullptr;
        }
        // TODO: bounds?
        auto *Alloca = builder.createAlloca(curDefType, value.getName());
        table.addVar(value.getName(), Alloca);
        auto *Val = visit(value.getValue());
        builder.createStore(Alloca, Val);
        return Alloca;
    }

    Value *visitBinExp(BinExp value) override {
        assert(!value.getOp().empty());
        auto *LHS = visit(value.getLeft());
        auto *RHS = visit(value.getRight());
        auto &Op = value.getOp();
        switch (Op[0]) {
            default: unreachable();
            case '+':
                return builder.createAdd(LHS, RHS);
            case '-':
                return builder.createSub(LHS, RHS);
            case '*':
                return builder.createMul(LHS, RHS);
            case '/':
                return builder.createDiv(LHS, RHS);
            case '%':
                return builder.createRem(LHS, RHS);
            case '<':
                return builder.createLt(LHS, RHS);
            case '>':
                return builder.createGt(LHS, RHS);
            case '=':
                if (Op == "==") {
                    return builder.createEq(LHS, RHS);
                } else {
                    diags.addError("unsupported operator: " + Op, value);
                }
                return nullptr;
            case '!':
                if (Op == "!=") {
                    return builder.createNe(LHS, RHS);
                } else {
                    diags.addError("unsupported operator: " + Op, value);
                }
                return nullptr;
            case '&':
                if (Op == "&&") {
                    return builder.createAnd(LHS, RHS);
                } else {
                    diags.addError("unsupported operator: " + Op, value);
                }
                return nullptr;
            case '|':
                if (Op == "||") {
                    return builder.createOr(LHS, RHS);
                } else {
                    diags.addError("unsupported operator: " + Op, value);
                }
                return nullptr;
        }
        return nullptr;
    }

    Value *visitExpStmt(ExpStmt value) override {
        return visit(value.getValue());
    }

    Value *visitAssignStmt(AssignStmt value) override {
        if (auto *LHS = visit(value.getLval())){
            auto *RHS = visit(value.getValue());
            builder.createStore(LHS, RHS);
            return RHS;
        }
        return nullptr;
    }

    Value *visitLVal(LVal value) override {
        auto *Alloca = table.getVar(value.getName());
        if (!Alloca) {
            diags.addError("undefined variable: " + value.getName(), value);
            return nullptr;
        }
        return Alloca;
    }

    Value *visitRVal(RVal value) override {
        auto *Alloca = table.getVar(value.getName());
        if (!Alloca) {
            diags.addError("undefined variable: " + value.getName(), value);
            return nullptr;
        }
        return builder.createLoad(Alloca);
    }

    Value *visitUnaExp(UnaExp value) override {
        unreachable();
        // TODO: implement unary expression
        return Visitor::visitUnaExp(value);
    }

    Value *visitDecLiteral(DecLiteral value) override {
        auto &Val = value.getValue();
        auto Int = std::stoi(Val);
        return context.getInt(Int);
    }

    Value *visitWhileStmt(WhileStmt value) override {
        auto *Cond = BasicBlock::Create(curFunc, "while.header");
        auto *Body = BasicBlock::Create(curFunc, "while.body");
        auto *Leave = BasicBlock::Create(curFunc, "while.leave");
        builder.createBr(Cond);
        builder.setInsertPoint(Cond);
        auto *CondVal = visit(value.getCond());
        builder.createCondBr(CondVal, Body, Leave);

        builder.setInsertPoint(Body);
        visit(value.getBody());
        builder.createBr(Cond);

        builder.setInsertPoint(Leave);
        return nullptr;
    }

    Value *visitIfElseStmt(IfElseStmt value) override {
        auto *Cond = BasicBlock::Create(curFunc, "if.cond");
        auto *Then = BasicBlock::Create(curFunc, "if.body");
        auto *Else = BasicBlock::Create(curFunc, "if.else");
        auto *Leave = BasicBlock::Create(curFunc, "if.leave");
        builder.createBr(Cond);
        builder.setInsertPoint(Cond);
        auto *CondVal = visit(value.getCond());
        builder.createCondBr(CondVal, Then, Else);

        builder.setInsertPoint(Then);
        visit(value.getThen());
        builder.createBr(Leave);

        builder.setInsertPoint(Else);
        visit(value.getElse());
        builder.createBr(Leave);

        builder.setInsertPoint(Leave);
        return nullptr;
    }

    Value *visitIfStmt(IfStmt value) override {
        auto *Cond = BasicBlock::Create(curFunc, "if.cond");
        auto *Then = BasicBlock::Create(curFunc, "if.then");
        auto *Leave = BasicBlock::Create(curFunc, "if.leave");
        builder.createBr(Cond);

        builder.setInsertPoint(Cond);
        auto *CondVal = visit(value.getCond());
        builder.createCondBr(CondVal, Then, Leave);

        builder.setInsertPoint(Then);
        visit(value.getThen());
        builder.createBr(Leave);

        builder.setInsertPoint(Leave);
        return nullptr;
    }

    Value *visitBlock(Block value) override {
        table.enterScope();
        visit(value.getStmts());
        table.leaveScope();
        return nullptr;
    }

    Value *visitReturnStmt(ReturnStmt value) override {
        if (value.getValue().empty()) {
            return builder.createRet();
        }
        auto *Val = visit(value.getValue());
        return builder.createRet(Val);
    }

};


#endif //DRAGON_CODEGEN_H
