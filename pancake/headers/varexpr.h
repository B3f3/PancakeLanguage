#ifndef VAREXPR_H
#define VAREXPR_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class VarExpr : public Expressions {        
    public:
        std::string name;
        explicit VarExpr(const std::string& name) : name(name) {}

        void debugPrint(int indent = 0) const override {
            std::cout << std::string(indent, ' ') << "VarExpr(" << name << ")\n";
        }
};


#endif //VAREXPR_H
