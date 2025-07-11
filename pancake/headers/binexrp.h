#ifndef BINEXPR_H
#define BINEXPR_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class BinExpr : public Expressions {        
    public:
        std::string op; // "+", "-", "*", "/", "mod"
        std::unique_ptr<Expressions> left;
        std::unique_ptr<Expressions> right;

        BinExpr(std::string op, std::unique_ptr<Expressions> left, std::unique_ptr<Expressions> right) 
            : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
};


#endif //BINEXPR_H
