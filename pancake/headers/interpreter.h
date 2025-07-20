#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <unordered_map>
#include <string>
#include <any>
#include <vector>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <queue>

#include "statements.h"
#include "expressions.h"

class Interpreter {
public:
    Interpreter() = default;

    // Main entry point to execute a program
    void execute(const std::vector<std::unique_ptr<Statements>>& statements);

private:
    std::unordered_map<std::string, std::any> variables;   // Variable environment (variable name -> value)
    std::queue<std::any> inputQueue;  // For feeding input in file mode

    // Execute a single statement
    void executeStatement(const Statements* stmt);

    // Evaluate an expression and return its result
    std::any evaluateExpression(const Expressions* expr);

    // Helpers to evaluate specific statement types
    void handleVarDecl(const class VarDecl* stmt);
    void handleAssignment(const class Assignment* stmt);
    void handleOut(const class OutStatement* stmt);
    void handleIn(const class InStatement* stmt);
    void handleIf(const class IfStatement* stmt);

    // Helpers to evaluate specific expression types
    std::any evaluateLiteral(const class Literal* expr);
    std::any evaluateVarExpr(const class VarExpr* expr);
    std::any evaluateBinExpr(const class BinExpr* expr);
    std::any evaluateUnaryExpr(const class UnaryExpr* expr);

    [[noreturn]] void runtimeError(const Statements* stmt, const std::string& msg);
    [[noreturn]] void runtimeError(const Expressions* expr, const std::string& msg);

};


#endif //INTERPRETER_H