//
// Created by Alex on 2022/3/19.
//

#ifndef DRAGON_SEMANTICANALYZER_H
#define DRAGON_SEMANTICANALYZER_H

#include "parser.h"
#include "Context.h"
#include "GlobalTable.h"

struct DiagLogger {
    virtual std::ostream &log() {
        return std::cout;
    }

    virtual std::ostream &err() {
        return std::cout;
    }
    virtual std::ostream &warn() {
        return std::cout;
    }

    DiagLogger() = default;
    virtual ~DiagLogger() = default;
};

struct SemanticAnalyzer : public Visitor<SemanticAnalyzer, Type *> {
    Context &context;
    DiagLogger &diag;
    GlobalTable globalTable;

    SemanticAnalyzer(Context &context, DiagLogger &diag) : context(context), diag(diag) {}

    Type *visitError(Error value) override {
        diag.err() << "Error: " << value.getContent() << std::endl;
        for (auto &Token: value.getValue()) {
            diag.err() << Token["lineStart"];
            diag.err() << Token["lineEnd"];
            diag.err() << Token["columnStart"];
            diag.err() << Token["columnEnd"];
            diag.err() << Token["lexeme"];
            diag.err() << std::endl;
        }
        return nullptr;
    }

    Type *visitProgram(Program value) override {
        return visit(value.getValue());
    }

    Type *visitFunctionDeclare(FunctionDeclare value) override {
        visit(value.getParams());
        return visit(value.getBlock());
    }

    Type *visitParamDef(ParamDef value) override {
        return nullptr;
    }

    Type *visitExprStmt(ExprStmt value) override {
        return visit(value.getValue());
    }

    Type *visitBlockStmt(BlockStmt value) override {
        globalTable.enterScope();
        auto *Ty = visit(value.getValue());
        globalTable.exitScope();
        return Ty;
    }

    Type *visitIfStmt(IfStmt value) override {
        if (!visit(value.getCondition())->isIntegerType()) {
            diag.err() << "Error: if condition must be integer type" << std::endl;
        }
        visit(value.getThen());
        return nullptr;
    }

    Type *visitWhileStmt(WhileStmt value) override {
        if (!visit(value.getCondition())->isIntegerType()) {
            diag.err() << "Error: while condition must be integer type" << std::endl;
        }
        visit(value.getBody());
        return nullptr;
    }

    Type *visitAssignExpr(AssignExpr value) override {
        auto *Left = visit(value.getLeft());
        auto *Right = visit(value.getRight());
        if (Left != Right){
            diag.err() << "Type mismatch in assignment" << std::endl;
        }
        return Left;
    }

    Type *visitInvokeExpr(InvokeExpr value) override {
        return Visitor::visitInvokeExpr(value);
    }

    Type *visitPtrSpecifier(PtrSpecifier value) override {
        auto *Ty = visit(value.getType());
        if (!Ty) {
            diag.err() << "Error: pointer type must be a valid type" << std::endl;
        }
        return Ty->getPointerType();
    }

    Type *visitTypeSpecifier(TypeSpecifier value) override {
        if (value.getTypeName() == "char") {
            return context.getInt8Ty();
        }
        if(value.getTypeName() == "int") {
            return context.getInt32Ty();
        }
        if (value.getTypeName() == "void") {
            return context.getVoidTy();
        }
        diag.err() << "Error: unknown type" << std::endl;
        return nullptr;
    }

    Type *visitCharLiteral(CharLiteral value) override {
        return context.getInt8Ty();
    }

    Type *visitFloatLiteral(FloatLiteral value) override {
        return context.getFloatTy();
    }

    Type *visitIntegerLiteral(IntegerLiteral value) override {
        return context.getInt32Ty();
    }

    Type *visitUnsignedLiteral(UnsignedLiteral value) override {
        return context.getInt32Ty();
    }

    Type *visitBinaryExpr(BinaryExpr value) override {
        Type *LHS = visit(value.getLeft());
        Type *RHS = visit(value.getRight());
        return Type::getMaxType(LHS, RHS);
    }

    Type *visitVariableDeclare(VariableDeclare value) override {
        auto *Ty = visit(value.getType());
        std::string Name = value.getName();
        auto *ValTy = visit(value.getInit());
        if (Ty != ValTy) {
            diag.err() << "Type mismatch in variable declaration" << std::endl;
        }
        return Ty;
    }

    Type *visitVariableExpr(VariableExpr value) override {
        return Visitor::visitVariableExpr(value);
    }

};


#endif //DRAGON_SEMANTICANALYZER_H
