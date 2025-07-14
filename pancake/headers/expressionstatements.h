#ifndef EXPRESSIONSTATEMENT_H
#define EXPRESSIONSTATEMENT_H

#include <memory>
#include <iostream>
#include "statements.h"
#include "expressions.h"

class ExpressionStatement : public Statements {
public:
    std::unique_ptr<Expressions> expression;

    ExpressionStatement(std::unique_ptr<Expressions> expr)
        : expression(std::move(expr)) {}

    void debugPrint(int indent = 0) const override {
        std::string ind(indent, ' ');
        std::cout << ind << "ExpressionStatement:\n";
        expression->debugPrint(indent + 2);
    }
};

#endif // EXPRESSIONSTATEMENT_H