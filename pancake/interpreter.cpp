#include <unordered_map>
#include <string>
#include <any>
#include <vector>
#include <memory>
#include <iostream>
#include <stdexcept>

#include "./headers/statements.h"
#include "./headers/expressions.h"
#include "./headers/interpreter.h"

#include "./headers/vardecl.h"
#include "./headers/ifstatement.h"
#include "./headers/instatement.h"
#include "./headers/outstatement.h"
#include "./headers/assignment.h"

#include "./headers/literal.h"
#include "./headers/binexrp.h"
#include "./headers/varexpr.h"

void Interpreter::execute(const std::vector<std::unique_ptr<Statements>>& statements) {
    for (const auto& stmt : statements) {
        executeStatement(stmt.get());
    }
}


void Interpreter::executeStatement(const Statements* stmt) {
    if (auto* v = dynamic_cast<const VarDecl*>(stmt)) handleVarDecl(v);
    else if (auto* a = dynamic_cast<const Assignment*>(stmt)) handleAssignment(a);
    else if (auto* o = dynamic_cast<const OutStatement*>(stmt)) handleOut(o);
    else if (auto* i = dynamic_cast<const InStatement*>(stmt)) handleIn(i);
    else if (auto* f = dynamic_cast<const IfStatement*>(stmt)) handleIf(f);
    else throw std::runtime_error("Unknown statement during execution.");
}


std::any Interpreter::evaluateExpression(const Expressions* expr) {
    if (auto* l = dynamic_cast<const Literal*>(expr)) return evaluateLiteral(l);
    if (auto* v = dynamic_cast<const VarExpr*>(expr)) return evaluateVarExpr(v);
    if (auto* b = dynamic_cast<const BinExpr*>(expr)) return evaluateBinExpr(b);
    
    throw std::runtime_error("Unknown expression type.");
}


void Interpreter::handleVarDecl(const VarDecl* stmt) {
    std::any value = evaluateExpression(stmt->value.get());
    variables[stmt->name] = value;
}


void Interpreter::handleAssignment(const Assignment* stmt) {
    if (variables.find(stmt->name) == variables.end()) {
        throw std::runtime_error("Assignment to undeclared variable: " + stmt->name);
    }
    variables[stmt->name] = evaluateExpression(stmt->value.get());
}


void Interpreter::handleOut(const OutStatement* stmt) {
    for (const auto& expr : stmt->outputs) {
        std::any result = evaluateExpression(expr.get());
        if (result.type() == typeid(int)) std::cout << std::any_cast<int>(result);
        else if (result.type() == typeid(double)) std::cout << std::any_cast<double>(result);
        else if (result.type() == typeid(bool)) std::cout << (std::any_cast<bool>(result) ? "true" : "false");
        else if (result.type() == typeid(std::string)) std::cout << std::any_cast<std::string>(result);
        else std::cout << "[unknown]";
    }
    std::cout << std::endl;
}


void Interpreter::handleIn(const InStatement* stmt) {
    std::string input;
    std::getline(std::cin, input);
    variables[stmt->varName] = input;  // You can cast to other types later
}


void Interpreter::handleIf(const IfStatement* stmt) {
    std::any cond = evaluateExpression(stmt->condition.get());
    bool isTrue = false;

    if (cond.type() == typeid(bool)) isTrue = std::any_cast<bool>(cond);
    else throw std::runtime_error("Condition must be a boolean");

    if (isTrue) {
        for (const auto& s : stmt->ifBranch) {
            executeStatement(s.get());
        }
    } else {
        bool executed = false;
        for (const auto& [elifCond, elifBranch] : stmt->elifBranches) {
            std::any elifResult = evaluateExpression(elifCond.get());
            if (elifResult.type() != typeid(bool)) continue;
            if (std::any_cast<bool>(elifResult)) {
                for (const auto& s : elifBranch) executeStatement(s.get());
                executed = true;
                break;
            }
        }
        if (!executed) {
            for (const auto& s : stmt->elseBranch) {
                executeStatement(s.get());
            }
        }
    }
}


std::any Interpreter::evaluateLiteral(const Literal* expr) {
    if (expr->type == "int") return std::stoi(expr->value);
    if (expr->type == "double") return std::stod(expr->value);
    if (expr->type == "bool") return expr->value == "true";
    if (expr->type == "string") return expr->value;
    throw std::runtime_error("Unknown literal type: " + expr->type);
}


std::any Interpreter::evaluateVarExpr(const VarExpr* expr) {
    auto it = variables.find(expr->name);
    if (it == variables.end()) {
        throw std::runtime_error("Undefined variable: " + expr->name);
    }
    return it->second;
}

std::any Interpreter::evaluateBinExpr(const BinExpr* expr) {
    auto left = evaluateExpression(expr->left.get());
    auto right = evaluateExpression(expr->right.get());

    if (left.type() == typeid(int) && right.type() == typeid(int)) {
        int l = std::any_cast<int>(left);
        int r = std::any_cast<int>(right);

        if (expr->op == "+") return l + r;
        if (expr->op == "-") return l - r;
        if (expr->op == "*") return l * r;
        if (expr->op == "/") return l / r;
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
        if (expr->op == "<") return l < r;
        if (expr->op == ">") return l > r;
        if (expr->op == "<=") return l <= r;
        if (expr->op == ">=") return l >= r;
    }

    throw std::runtime_error("Unsupported binary operation or mismatched types.");
}