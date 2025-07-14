#ifndef IFSTATEMENT_H
#define IFSTATEMENT_H

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

    void debugPrint(int indent = 0) const override {
    std::string ind(indent, ' ');
    std::cout << ind << "IfStatement:\n";

    std::cout << ind << "  Condition:\n";
    condition->debugPrint(indent + 4);

    std::cout << ind << "  If block:\n";
    for (const auto& stmt : elseBranch) {
        stmt->debugPrint(indent + 4);
    }

    for (const auto& [elifCond, elifStmts] : elifBranches) {
        std::cout << ind << "  ElseIf condition:\n";
        elifCond->debugPrint(indent + 4);
        std::cout << ind << "  ElseIf block:\n";
        for (const auto& stmt : elifStmts) {
            stmt->debugPrint(indent + 4);
        }
    }

    if (!elseBranch.empty()) {
        std::cout << ind << "  Else block:\n";
        for (const auto& stmt : elseBranch) {
            stmt->debugPrint(indent + 4);
        }
    }
}

};


#endif //IFSTATEMENT_H