#ifndef OUTSTATEMENT_H
#define OUTSTATEMENT_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class OutStatement : public Statements {
public:
    std::vector<std::unique_ptr<Expressions>> outputs;

    explicit OutStatement(std::vector<std::unique_ptr<Expressions>> outputs)
        : outputs(std::move(outputs)) {}
};

#endif //OUTSTATEMENT_H