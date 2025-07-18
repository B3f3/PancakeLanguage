#include "./headers/parser.h"
#include "./headers/token.h"      // for Token
#include "./headers/typechecker.h"

#include "./headers/statements.h" // for Statements and subclasses
#include "./headers/vardecl.h"
#include "./headers/ifstatement.h"
#include "./headers/instatement.h"
#include "./headers/outstatement.h"
#include "./headers/assignment.h"

#include "./headers/expressions.h" // for Expressions and subclasses
#include "./headers/literal.h"
#include "./headers/binexrp.h"
#include "./headers/varexpr.h"

#include <stdexcept>

const int LOWEST_PRECEDENCE = 1;

Parser::Parser(const std::vector<Token>& tokens, TypeChecker& typeChecker)
    : tokens(tokens), typeChecker(typeChecker), current(0) {}

    
std::vector<std::unique_ptr<Statements>> Parser::parse() {
    std::vector<std::unique_ptr<Statements>> statements;
    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {  // Only add if not null
            statements.push_back(std::move(stmt));
        }
    }
    return statements;
}


std::unique_ptr<Statements> Parser::parseStatement() {
    // Skip any empty lines
    while (match(TokenType::END_OF_LINE)) {}
    
    if (isAtEnd() || check(TokenType::RBRACE)) {
        return nullptr;  // Return null for EOF or closing brace
    }
    
    // Handle actual statements
    if (check(TokenType::LET)) return parseVarDecl();
    if (match(TokenType::OUT)) return parseOut();
    if (match(TokenType::IN)) return parseIn();
    if (match(TokenType::IF)) return parseIf();
    if (peek().type == TokenType::IDENTIFIER && peekNext().type == TokenType::EQ) {
        return parseExpressionStatement();
    }

    error(peek(), "Unknown Statement: " + peek().value);
}

std::unique_ptr<Statements> Parser::parseVarDecl() {
    consume(TokenType::LET, "Expected 'let' keyword");
    std::string type;
    if (peek().type == TokenType::TYPE_INT) type = "int";
    else if (peek().type == TokenType::TYPE_DOUBLE) type = "double";
    else if (peek().type == TokenType::TYPE_STRING) type = "string";
    else if (peek().type == TokenType::TYPE_BOOL) type = "bool";
    else error(peek(), "Expected variable type");

    advance();

    if (peek().type != TokenType::IDENTIFIER)
        error(peek(), "Expected variable name");

    std::string name = peek().value;
    advance();

    consume(TokenType::EQ, "Expected '=' in variable declaration");
    auto value = parseExpression();

    // Check type match
    if (auto* literal = dynamic_cast<Literal*>(value.get())) {
        if (literal->type != type) {
            throw std::runtime_error("Type mismatch: expected " + type + " but got " + literal->type);
        }
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    // Register in type checker
    typeChecker.declare(name, type);

    return std::make_unique<VarDecl>(type, name, std::move(value));
}


std::unique_ptr<Statements> Parser::parseOut() {
    std::vector<std::unique_ptr<Expressions>> expressions;

    do {
        consume(TokenType::ARROWF, "Expected '->' after 'out' or expression");
        expressions.push_back(parseExpression());
    } while (check(TokenType::ARROWF)); // Keep going while more arrows

    consume(TokenType::SEMICOLON, "Expected ';' after out statement");

    return std::make_unique<OutStatement>(std::move(expressions));
}


std::unique_ptr<Statements> Parser::parseIn() {

    consume(TokenType::ARROWB, "Expected '<-' after 'in'");

    if (peek().type != TokenType::IDENTIFIER)
        throw std::runtime_error("Expected identifier after '<-' in input statement");

    std::string name = peek().value;
    advance();

    consume(TokenType::SEMICOLON, "Expected ';' after input statement");

    return std::make_unique<InStatement>(name);
}


std::unique_ptr<Statements> Parser::parseIf() {
    // Parse initial `if` condition
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    // Parse main 'if' block
    consume(TokenType::LBRACE, "Expected '{' after if condition");
    std::vector<std::unique_ptr<Statements>> ifBranch;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        ifBranch.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after if block");

    // Parse `else if` branches
    std::vector<std::pair<std::unique_ptr<Expressions>, std::vector<std::unique_ptr<Statements>>>> elifBranches;
    while (match(TokenType::ELSE)) {
        if (match(TokenType::IF)) {
            // Parse `else if (...) { ... }`
            consume(TokenType::LPAREN, "Expected '(' after 'else if'");
            auto elifCondition = parseExpression();
            consume(TokenType::RPAREN, "Expected ')' after condition");

            consume(TokenType::LBRACE, "Expected '{' after 'else if' condition");
            std::vector<std::unique_ptr<Statements>> elifBlock;
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                elifBlock.push_back(parseStatement());
            }
            consume(TokenType::RBRACE, "Expected '}' after 'else if' block");

            elifBranches.emplace_back(std::move(elifCondition), std::move(elifBlock));
        } else {
            // Handle final `else { ... }` block
            consume(TokenType::LBRACE, "Expected '{' after 'else'");
            std::vector<std::unique_ptr<Statements>> elseBlock;
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                elseBlock.push_back(parseStatement());
            }
            consume(TokenType::RBRACE, "Expected '}' after 'else' block");

        return std::make_unique<IfStatement>(
            std::move(condition),
            std::move(ifBranch),
            std::move(elifBranches),
            std::move(elseBlock)
        );

        }
    }

    // If no final `else`:
    return std::make_unique<IfStatement>(
        std::move(condition),
        std::move(ifBranch),
        std::move(elifBranches),
        std::vector<std::unique_ptr<Statements>>{}  // empty elseBranch
    );
}


std::unique_ptr<Statements> Parser::parseExpressionStatement() {
    // Only allow assignments like: x = expression;
    if (peek().type == TokenType::IDENTIFIER && peekNext().type == TokenType::EQ) {
        std::string name = peek().value;
        advance(); // consume identifier
        advance(); // consume '='
        auto value = parseExpression();

        // Type check
        std::string expectedType = typeChecker.getType(name);
        if (expectedType.empty()) {
            throw std::runtime_error("Assignment to undeclared variable: " + name);
        }

        // Literal check
        if (auto* literal = dynamic_cast<Literal*>(value.get())) {
            if (literal->type != expectedType) {
                throw std::runtime_error("Type mismatch: variable '" + name + "' expects " + expectedType + " but got " + literal->type);
            }
        }

        consume(TokenType::SEMICOLON, "Expected ';' after assignment");

        return std::make_unique<Assignment>(name, std::move(value));
    }

    throw std::runtime_error("Invalid expression statement");
}



std::unique_ptr<Expressions> Parser::parseExpression(){
    auto leftHandSide = parsePrimary();
    return parseBinary(std::move(leftHandSide), LOWEST_PRECEDENCE);
}


std::unique_ptr<Expressions> Parser::parsePrimary() {
    Token token = peek();

    if (token.type == TokenType::INT_LITERAL) {
        std::string value = token.value;
        advance();
        return std::make_unique<Literal>(value, "int");
    }

    if (token.type == TokenType::DOUBLE_LITERAL) {
        std::string value = token.value;
        advance();
        return std::make_unique<Literal>(value, "double");
    }

    if (token.type == TokenType::STRING_LITERAL) {
        std::string value = token.value;
        advance();
        return std::make_unique<Literal>(value, "string");
    }

    if (token.type == TokenType::BOOL_LITERAL) {
        std::string value = token.value;
        advance();
        return std::make_unique<Literal>(value, "bool");
    }

    if (token.type == TokenType::IDENTIFIER) {
        std::string name = token.value;
        advance();
        return std::make_unique<VarExpr>(name);
    }

    if (token.type == TokenType::LPAREN) {
        advance(); // consume '('
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    throw std::runtime_error("Unexpected token in expression");
}


std::unique_ptr<Expressions> Parser::parseBinary(std::unique_ptr<Expressions> left, int minPrecedence) {
    while (true) {
        int precedence = getPrecedence(peek());
        if (precedence < minPrecedence) break;

        std::string op = peek().value;
        advance();

        // Right expression with higher precedence
        auto right = parsePrimary();

        int nextPrecedence = getPrecedence(peek());
        if (precedence < nextPrecedence) {
            right = parseBinary(std::move(right), precedence + 1);
        }

        left = std::make_unique<BinExpr>(op, std::move(left), std::move(right));
    }
    return left;
}


bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return !isAtEnd() && peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::peekNext() const {
    if (current + 1 < tokens.size()) {
        return tokens[current + 1];
    }
    return tokens.back(); // or some dummy EOF token
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

void Parser::consume(TokenType type, const std::string& errorMsg) {
    if (!match(type)) throw std::runtime_error(errorMsg);
}

int Parser::getPrecedence(const Token& tok){
    switch (tok.type) {
        case TokenType::EE:
        case TokenType::NE:
            return 1; // Equality operators (x == y, x != y)

        case TokenType::LT:
        case TokenType::LTE:
        case TokenType::GT:
        case TokenType::GTE:
            return 2; // Comparison operators (x < y, x >= y)

        case TokenType::PLUS:
        case TokenType::MINUS:
            return 3;

        case TokenType::MUL:
        case TokenType::DIV:
        case TokenType::MOD:
            return 4;

        default:
            return 0;
    }
}

[[noreturn]] void Parser::error(const Token& token, const std::string& message) const {
    throw std::runtime_error(
        "Syntax Error at line " + std::to_string(token.line) +
        ", column " + std::to_string(token.column) +
        ": " + message + " " + token.value
    );
}
