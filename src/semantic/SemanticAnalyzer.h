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

};


#endif //DRAGON_SEMANTICANALYZER_H
