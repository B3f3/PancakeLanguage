#ifndef INSTATEMENT_H
#define INSTATEMENT_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class InStatement : public Statements {
public:
    std::string varName;
    explicit InStatement(const std::string& name) : varName(name) {}

    void debugPrint(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "InStatement(" << varName << ")\n";
    }
};

#endif //INSTATEMENT_H
