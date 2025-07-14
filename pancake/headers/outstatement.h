#ifndef OUTSTATE_H
#define OUTSTATE_H

#include <memory>
#include <vector>
#include "statements.h"
#include "expressions.h"

class OutStatement : public Statements {
public:
    std::vector<std::unique_ptr<Expressions>> outputs;

    explicit OutStatement(std::unique_ptr<Expressions> expr) {
        outputs.push_back(std::move(expr));
    }

    // Optional: constructor for multiple expressions
    explicit OutStatement(std::vector<std::unique_ptr<Expressions>> exprs)
        : outputs(std::move(exprs)) {}

    void debugPrint(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "OutStatement\n";
        for (const auto& expr : outputs) {
            expr->debugPrint(indent + 2);
        }
    }
};


#endif


