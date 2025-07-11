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
};

#endif //INSTATEMENT_H
