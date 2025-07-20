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
#include "./headers/unaryexpr.h"

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
    else runtimeError(stmt, "Unknown statement during execution");
}


std::any Interpreter::evaluateExpression(const Expressions* expr) {
    if (auto* l = dynamic_cast<const Literal*>(expr)) return evaluateLiteral(l);
    if (auto* v = dynamic_cast<const VarExpr*>(expr)) return evaluateVarExpr(v);
    if (auto* b = dynamic_cast<const BinExpr*>(expr)) return evaluateBinExpr(b);
    if (auto* u = dynamic_cast<const UnaryExpr*>(expr)) return evaluateUnaryExpr(u);  // Add this line
    
    runtimeError(expr, "Unknown expression type.");
}


void Interpreter::handleVarDecl(const VarDecl* stmt) {
    if (variables.find(stmt->name) != variables.end()) {
        runtimeError(stmt, "Variable already declared: " + stmt->name);
    }
    std::any value = evaluateExpression(stmt->value.get());
    variables[stmt->name] = value;
}


void Interpreter::handleAssignment(const Assignment* stmt) {
    if (variables.find(stmt->name) == variables.end()) {
        runtimeError(stmt, "Assignment to undeclared variable: " + stmt->name);
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
    // Check if we have predefined input (for file mode)
    if (!inputQueue.empty()) {
        variables[stmt->varName] = inputQueue.front();
        inputQueue.pop();
        return;
    }
    
    // Interactive mode
    std::string input;
    if (!std::getline(std::cin, input)) {
        runtimeError(stmt, "Failed to read input");
    }
    
    // Convert based on variable type if known
    if (variables.count(stmt->varName)) {
        auto& var = variables[stmt->varName];
        if (var.type() == typeid(int)) {
            var = std::stoi(input);
        } else if (var.type() == typeid(double)) {
            var = std::stod(input);
        } else {
            var = input;
        }
    } else {
        variables[stmt->varName] = input;
    }
}


void Interpreter::handleIf(const IfStatement* stmt) {
    std::any cond = evaluateExpression(stmt->condition.get());
    bool isTrue = false;

    if (cond.type() == typeid(bool)) isTrue = std::any_cast<bool>(cond);
    else runtimeError(stmt, "Condition must be a boolean");

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
    runtimeError(expr, "Unknown literal type: " + expr->type);
}


std::any Interpreter::evaluateVarExpr(const VarExpr* expr) {
    auto it = variables.find(expr->name);
    if (it == variables.end()) {
        runtimeError(expr, "Undefined variable: " + expr->name);
    }
    return it->second;
}

std::any Interpreter::evaluateBinExpr(const BinExpr* expr) {
    // First evaluate the left operand
    auto left = evaluateExpression(expr->left.get());

    // For binary operators, evaluate the right operand
    auto right = evaluateExpression(expr->right.get());

    // Helper function to get numeric value as double
    auto get_as_double = [](const std::any& value) {
        if (value.type() == typeid(int)) return static_cast<double>(std::any_cast<int>(value));
        if (value.type() == typeid(double)) return std::any_cast<double>(value);
        throw std::runtime_error("Numeric value expected");
    };

    // Integer operations
    if (left.type() == typeid(int) && right.type() == typeid(int)) {
        int l = std::any_cast<int>(left);
        int r = std::any_cast<int>(right);

        if (expr->op == "+") return l + r;
        if (expr->op == "-") return l - r;
        if (expr->op == "*") return l * r;
        if (expr->op == "/") {
            if (r == 0) runtimeError(expr, "Division by zero");
            return l / r;  // Integer division
        }
        if (expr->op == "mod") {
            if (r == 0) runtimeError(expr, "Modulo by zero");
            return l % r;
        }
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
        if (expr->op == "<") return l < r;
        if (expr->op == ">") return l > r;
        if (expr->op == "<=") return l <= r;
        if (expr->op == ">=") return l >= r;
    }
    // Double operations
    else if (left.type() == typeid(double) && right.type() == typeid(double)) {
        double l = std::any_cast<double>(left);
        double r = std::any_cast<double>(right);

        if (expr->op == "+") return l + r;
        if (expr->op == "-") return l - r;
        if (expr->op == "*") return l * r;
        if (expr->op == "/") {
            if (r == 0.0) runtimeError(expr, "Division by zero");
            return l / r;
        }
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
        if (expr->op == "<") return l < r;
        if (expr->op == ">") return l > r;
        if (expr->op == "<=") return l <= r;
        if (expr->op == ">=") return l >= r;
    }
    // Mixed numeric types (int + double)
    else if ((left.type() == typeid(int) || left.type() == typeid(double)) &&
             (right.type() == typeid(int) || right.type() == typeid(double))) {
        double l = get_as_double(left);
        double r = get_as_double(right);

        if (expr->op == "+") return l + r;
        if (expr->op == "-") return l - r;
        if (expr->op == "*") return l * r;
        if (expr->op == "/") {
            if (r == 0.0) runtimeError(expr, "Division by zero");
            return l / r;
        }
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
        if (expr->op == "<") return l < r;
        if (expr->op == ">") return l > r;
        if (expr->op == "<=") return l <= r;
        if (expr->op == ">=") return l >= r;
    }
    // String concatenation
    else if (expr->op == "+" && 
             left.type() == typeid(std::string) && 
             right.type() == typeid(std::string)) {
        return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
    }
    // String equality
    else if ((expr->op == "==" || expr->op == "!=") &&
             left.type() == typeid(std::string) && 
             right.type() == typeid(std::string)) {
        bool equal = std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
        return expr->op == "==" ? equal : !equal;
    }
    // Boolean operations
    else if (left.type() == typeid(bool) && right.type() == typeid(bool)) {
        bool l = std::any_cast<bool>(left);
        bool r = std::any_cast<bool>(right);

        if (expr->op == "and" ) return l && r;
        if (expr->op == "or" ) return l || r;
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
    }

    runtimeError(expr, "Unsupported operation '" + expr->op + "' for types " + 
        left.type().name() + " and " + right.type().name());
}

std::any Interpreter::evaluateUnaryExpr(const class UnaryExpr* expr) {
    auto operand = evaluateExpression(expr->getExpr());
    const std::string& op = expr->getOp();

    if (op == "!") {
        // Handle logical NOT
        if (operand.type() == typeid(bool)) {
            return !std::any_cast<bool>(operand);
        }
        runtimeError(expr, "NOT operator '!' requires a boolean operand");
    }
    else if (op == "-") {
        // Handle unary minus
        if (operand.type() == typeid(int)) {
            return -std::any_cast<int>(operand);
        }
        if (operand.type() == typeid(double)) {
            return -std::any_cast<double>(operand);
        }
        runtimeError(expr, "Unary minus '-' requires a numeric operand");
    }

    runtimeError(expr, "Unknown unary operator: " + op);
}


[[noreturn]] void Interpreter::runtimeError(const Statements* stmt, const std::string& msg) {
    throw std::runtime_error("Runtime Error at line " + std::to_string(stmt->line) + 
                             ", column " + std::to_string(stmt->column) + ": " + msg);
}

[[noreturn]] void Interpreter::runtimeError(const Expressions* expr, const std::string& msg) {
    throw std::runtime_error("Runtime Error at line " + std::to_string(expr->line) + 
                             ", column " + std::to_string(expr->column) + ": " + msg);
}
