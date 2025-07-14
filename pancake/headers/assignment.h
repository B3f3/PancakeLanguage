#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include "statements.h"
#include "expressions.h"
#include <string>
#include <memory>
#include <iostream>

class Assignment : public Statements {
public:
    std::string name;
    std::unique_ptr<Expressions> value;

    Assignment(const std::string& name, std::unique_ptr<Expressions> value)
        : name(name), value(std::move(value)) {}

    void debugPrint(int indent = 0) const override {
        std::string ind(indent, ' ');
        std::cout << ind << "Assignment:\n";
        std::cout << ind << "  Var: " << name << "\n";
        std::cout << ind << "  Value:\n";
        value->debugPrint(indent + 4);
    }
};

#endif