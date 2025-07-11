#ifndef OUTSTATEMENT_H
#define OUTSTATEMENT_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class IfStatement : public Statements {
public:
    std::unique_ptr<Expressions> condition;
    std::vector<std::pair<std::unique_ptr<Expressions>, std::vector<std::unique_ptr<Statements>>>> elifBranches;
    std::vector<std::unique_ptr<Statements>> elseBranch;

    IfStatement(std::unique_ptr<Expressions> cond,
           std::vector<std::pair<std::unique_ptr<Expressions>, std::vector<std::unique_ptr<Statements>>>> elifBranches,
           std::vector<std::unique_ptr<Statements>> elseBranch)
        : condition(std::move(cond)),
          elifBranches(std::move(elifBranches)),
          elseBranch(std::move(elseBranch)) {}
};


#endif //OUTSTATEMENT_H