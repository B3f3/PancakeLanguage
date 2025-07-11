#ifndef VARDECL_H
#define VARDECL_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class VarDecl : public Statements {
public:
    std::string type;  // "int", "double", etc.
    std::string name;
    std::unique_ptr<Expressions> value;

    VarDecl(const std::string& type, const std::string& name, std::unique_ptr<Expressions> value)
        : type(type), name(name), value(std::move(value)) {}
};

#endif //VARDECL_H
