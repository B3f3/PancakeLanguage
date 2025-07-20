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
#include "./headers/unaryexpr.h"

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
    // Skip any empty lines or unexpected tokens before statements
    while (!isAtEnd()) {
        // Accept these as valid "empty" statements
        if (match(TokenType::END_OF_LINE) || match(TokenType::SEMICOLON)) {
            continue;
        }
        // Stop skipping when we hit a closing brace
        if (check(TokenType::RBRACE)) {
            return nullptr;
        }
        break;
    }

    if (isAtEnd()) {
        return nullptr;
    }

    // Handle actual statements
    if (check(TokenType::LET)) return parseVarDecl();
    if (match(TokenType::OUT)) return parseOut();
    if (match(TokenType::IN)) return parseIn();
    if (match(TokenType::IF)) return parseIf();
    if (peek().type == TokenType::IDENTIFIER && peekNext().type == TokenType::EQ) {
        return parseExpressionStatement();
    }

    throw std::runtime_error( "Unexpected Statement: '"  + peek().value + "' at line " 
        + std::to_string(peek().line) +
        ", column " + std::to_string(peek().column));
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
            error(peek(), "Type mismatch: expected " + type + " but got " + literal->type);
        }
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    // Register in type checker
    typeChecker.declare(name, type);

    auto varDecl = std::make_unique<VarDecl>(type, name, std::move(value));
    varDecl->column = peek().column;
    varDecl->line = peek().line;
    return varDecl;
}


std::unique_ptr<Statements> Parser::parseOut() {
    std::vector<std::unique_ptr<Expressions>> expressions;

    do {
        consume(TokenType::ARROWF, "Expected '->' after 'out' or expression");
        expressions.push_back(parseExpression());
    } while (check(TokenType::ARROWF)); // Keep going while more arrows

    consume(TokenType::SEMICOLON, "Expected ';' after out statement");

    auto outStmt = std::make_unique<OutStatement>(std::move(expressions));
    outStmt->column = peek().column;
    outStmt->line = peek().line;
    return outStmt;
}


std::unique_ptr<Statements> Parser::parseIn() {

    consume(TokenType::ARROWB, "Expected '<-' after 'in'");

    if (peek().type != TokenType::IDENTIFIER)
        error(peek(), "Expected identifier after '<-' in input statement");

    std::string name = peek().value;
    advance();

    consume(TokenType::SEMICOLON, "Expected ';' after input statement");

    auto inStmt = std::make_unique<InStatement>(name);
    inStmt->column = peek().column;
    inStmt->line = peek().line;
    return inStmt;
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
        auto stmt = parseStatement();
        stmt->column = peek().column;
        stmt->line = peek().line;
        if (stmt) ifBranch.push_back(std::move(stmt));
    }
    consume(TokenType::RBRACE, "Expected '}' after if block");

    // Parse `elif` branches
    std::vector<std::pair<std::unique_ptr<Expressions>, std::vector<std::unique_ptr<Statements>>>> elifBranches;
    while (match(TokenType::ELIF)) {
        consume(TokenType::LPAREN, "Expected '(' after 'elif'");
        auto elifCondition = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after condition");

        consume(TokenType::LBRACE, "Expected '{' after 'elif' condition");
        std::vector<std::unique_ptr<Statements>> elifBlock;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            auto stmt = parseStatement();
            stmt->column = peek().column;
            stmt->line = peek().line;
            if (stmt) elifBlock.push_back(std::move(stmt));
        }
        consume(TokenType::RBRACE, "Expected '}' after 'elif' block");

        elifBranches.emplace_back(std::move(elifCondition), std::move(elifBlock));
    }

    // Parse else branch (if present)
    std::vector<std::unique_ptr<Statements>> elseBranch;
    if (match(TokenType::ELSE)) {
        consume(TokenType::LBRACE, "Expected '{' after 'else'");
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            auto stmt = parseStatement();
            stmt->column = peek().column;
            stmt->line = peek().line;
            if (stmt) elseBranch.push_back(std::move(stmt));
        }
        consume(TokenType::RBRACE, "Expected '}' after 'else' block");
    }

    auto ifStmt = std::make_unique<IfStatement>(
        std::move(condition),
        std::move(ifBranch),
        std::move(elifBranches),
        std::move(elseBranch)
    );
    ifStmt->column = peek().column;
    ifStmt->line = peek().line;
    return ifStmt;
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
            error(peek(), "Assignment to undeclared variable: " + name);
        }

        // Literal check
        if (auto* literal = dynamic_cast<Literal*>(value.get())) {
            if (literal->type != expectedType) {
                error(peek(), "Type mismatch: variable '" + name + "' expects " + expectedType + " but got " + literal->type);
            }
        }

        consume(TokenType::SEMICOLON, "Expected ';' after assignment");
        auto assignment = std::make_unique<Assignment>(name, std::move(value));
        assignment->column = peek().column;
        assignment->line = peek().line;
        return assignment;
    }
    error(peek(),"Invalid expression statement");
}



std::unique_ptr<Expressions> Parser::parseExpression() {
    if (match(TokenType::NOT)) {
        Token op = previous(); // capture '!'
        auto expr = parseExpression();
        auto unexpression = std::make_unique<UnaryExpr>("!", std::move(expr));
        unexpression->line = op.line;
        unexpression->column = op.column;
        return unexpression;
    }

    if (match(TokenType::MINUS)) {
        Token op = previous(); // capture '-'
        auto expr = parseExpression();
        auto unexpression = std::make_unique<UnaryExpr>("-", std::move(expr));
        unexpression->line = op.line;
        unexpression->column = op.column;
        return unexpression;
    }

    auto leftHandSide = parsePrimary();
    return parseBinary(std::move(leftHandSide), LOWEST_PRECEDENCE);
}



std::unique_ptr<Expressions> Parser::parsePrimary() {
    Token token = peek();

    if (token.type == TokenType::INT_LITERAL) {
        std::string value = token.value;
        advance();
        auto lit = std::make_unique<Literal>(value, "int");
        lit->column = peek().column;
        lit->line = peek().line;
        return lit;
    }

    if (token.type == TokenType::DOUBLE_LITERAL) {
        std::string value = token.value;
        advance();
        auto lit = std::make_unique<Literal>(value, "double");
        lit->column = peek().column;
        lit->line = peek().line;
        return lit;
    }

    if (token.type == TokenType::STRING_LITERAL) {
        std::string value = token.value;
        advance();
        auto lit = std::make_unique<Literal>(value, "string");
        lit->column = peek().column;
        lit->line = peek().line;
        return lit;
    }

    if (token.type == TokenType::BOOL_LITERAL) {
        std::string value = token.value;
        advance();
        auto lit = std::make_unique<Literal>(value, "bool");
        lit->column = peek().column;
        lit->line = peek().line;
        return lit;
    }

    if (token.type == TokenType::IDENTIFIER) {
        std::string name = token.value;
        advance();
        auto varExp = std::make_unique<VarExpr>(name);
        varExp->column = peek().column;
        varExp->line = peek().line;
        return varExp;
    }

    if (token.type == TokenType::LPAREN) {
        advance(); // consume '('
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    error(peek(), "Unexpected token in expression");
}


std::unique_ptr<Expressions> Parser::parseBinary(std::unique_ptr<Expressions> left, int minPrecedence) {
    while (true) {
        int precedence = getPrecedence(peek());
        if (precedence < minPrecedence) break;

        Token opToken = peek();  // Save token for line/col
        std::string op = opToken.value;
        advance();

        auto right = parsePrimary();
        int nextPrecedence = getPrecedence(peek());
        if (precedence < nextPrecedence) {
            right = parseBinary(std::move(right), precedence + 1);
        }

        auto binExpr = std::make_unique<BinExpr>(op, std::move(left), std::move(right));
        binExpr->line = opToken.line;
        binExpr->column = opToken.column;
        left = std::move(binExpr);
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
    if (!match(type)) error(peek(), errorMsg);
}

int Parser::getPrecedence(const Token& tok) {
    switch (tok.type) {
        case TokenType::NOT:
            return 6; // Highest precedence for 'not'
            
        case TokenType::EE:
        case TokenType::NE:
            return 1; // Equality operators
            
        case TokenType::LT:
        case TokenType::LTE:
        case TokenType::GT:
        case TokenType::GTE:
            return 2; // Comparison operators
            
        case TokenType::AND:
            return 3; // Logical AND
            
        case TokenType::OR:
            return 4; // Logical OR
            
        case TokenType::PLUS:
        case TokenType::MINUS:
            return 5;
            
        case TokenType::MUL:
        case TokenType::DIV:
        case TokenType::MOD:
            return 6;
            
        default:
            return 0;
    }
}

[[noreturn]] void Parser::error(const Token& token, const std::string& message) const {
    throw std::runtime_error(
        "Syntax Error at line " + std::to_string(token.line) +
        ", column " + std::to_string(token.column) +
        ": " + message + " got '" + token.value + "' "
    );
}
