#ifndef UNARYEXPR_H
#define UNARYEXPR_H

#include "expressions.h"
#include <memory>
#include <string>
#include <iostream>

class UnaryExpr : public Expressions {
    std::string op;
    std::unique_ptr<Expressions> expr;

public:
    UnaryExpr(std::string op, std::unique_ptr<Expressions> expr)
        : op(std::move(op)), expr(std::move(expr)) {}

    const std::string& getOp() const { return op; }
    const Expressions* getExpr() const { return expr.get(); }

    void debugPrint(int indent = 0) const override {
        std::string pad(indent, ' ');
        std::cout << pad << "UnaryExpr(" << op << ")\n";
        expr->debugPrint(indent + 2);
    }
};

#endif //UNARYEXPR_H
