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
};


#endif //VAREXPR_H
