#include "./headers/parser.h"
#include "./headers/token.h"      // for Token

#include "./headers/statements.h" // for Statements and subclasses
#include "./headers/vardecl.h"
#include "./headers/ifstatement.h"
#include "./headers/instatement.h"
#include "./headers/outstatement.h"

#include "./headers/expressions.h" // for Expressions and subclasses
#include "./headers/literal.h"
#include "./headers/binexrp.h"
#include "./headers/varexpr.h"

#include <stdexcept>

const int LOWEST_PRECEDENCE = 1;

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

    
std::vector<std::unique_ptr<Statements>> Parser::parse() {
    std::vector<std::unique_ptr<Statements>> statements;
    while (!isAtEnd()) {
        statements.push_back(parseStatement());
    }
    return statements;
}


std::unique_ptr<Statements> Parser::parseStatement() {
    if (match(TokenType::LET)) return parseVarDecl();
    if (match(TokenType::OUT)) return parseOut();
    if (match(TokenType::IN))  return parseIn();
    if (match(TokenType::IF))  return parseIf();

    throw std::runtime_error("Unknown statement.");
}

std::unique_ptr<Statements> Parser::parseVarDecl(){    
    std::string type;
    if (peek().type == TokenType::TYPE_INT) type = "int";
    else if (peek().type == TokenType::TYPE_DOUBLE) type = "double";
    else if (peek().type == TokenType::TYPE_STRING) type = "string";
    else if (peek().type == TokenType::TYPE_BOOL) type = "bool";
    else throw std::runtime_error("Expected variable type");

    advance();

    if (peek().type != TokenType::IDENTIFIER)
        throw std::runtime_error("Expected variable name");
    std::string name = peek().value;
    advance();

    consume(TokenType::EQ, "Expected '=' in variable declaration");

    auto value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
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

std::unique_ptr<Expressions> Parser::parseExpression(){
    auto leftHandSide = parsePrimary();
    return parseBinary(std::move(leftHandSide), LOWEST_PRECEDENCE);
}


std::unique_ptr<Expressions> Parser::parsePrimary(){
    if (peek().type == TokenType::INT_LITERAL || peek().type == TokenType::STRING_LITERAL || peek().type == TokenType::BOOL_LITERAL || peek().type == TokenType::DOUBLE_LITERAL)
    {
        std::string value = peek().value;
        advance();
        return std::make_unique<Literal>(value);
    }
    if (peek().type == TokenType::IDENTIFIER){
        std::string name = peek().value;
        advance();
        return std::make_unique<VarExpr>(name);
    }
    if (peek().type == TokenType::LPAREN)
    {
        advance(); // consume '('
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    throw std::runtime_error("Unexpected token in expression");
    
}


std::unique_ptr<Expressions> Parser::parseBinary(std::unique_ptr<Expressions> left, int minPrecedence){

    while (true)
    {
        int tokenPrecedence = getPrecedence(peek());
        if (tokenPrecedence < minPrecedence) return left;

        std::string op = peek().value;
        advance();
        auto right = parsePrimary();

         int nextPrecedence = getPrecedence(peek());
         if (tokenPrecedence < nextPrecedence) {
            right = parseBinary(std::move(right), tokenPrecedence + LOWEST_PRECEDENCE);
        }
        left = std::make_unique<BinExpr>(op, std::move(left), std::move(right));
    }
    return left;
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
                std::move(elifBranches),
                std::move(elseBlock)
            );
        }
    }

    // If no final `else`:
    return std::make_unique<IfStatement>(
        std::move(condition),
        std::move(elifBranches),
        std::vector<std::unique_ptr<Statements>>{}  // empty elseBranch
    );
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
        case TokenType::PLUS:
        case TokenType::MINUS:
            return 1;

        case TokenType::MUL:
        case TokenType::DIV:
        case TokenType::MOD:
            return 2;

        default:
            return 0; // Not a binary operator
    }
}