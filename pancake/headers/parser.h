#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "token.h"      // for Token
#include "statements.h" // for Statements and subclasses
#include "expressions.h"// for Expressions and subclasses

class Parser
{
private:
    const std::vector<Token>& tokens;
    size_t current;

    //Statement core functions
    std::unique_ptr<Statements> parseStatement();
    std::unique_ptr<Statements> parseVarDecl();
    std::unique_ptr<Statements> parseOut();
    std::unique_ptr<Statements> parseIn();
    std::unique_ptr<Statements> parseIf();

    //Expression core functions
    std::unique_ptr<Expressions> parseExpression();
    std::unique_ptr<Expressions> parsePrimary();
    std::unique_ptr<Expressions> parseBinary(std::unique_ptr<Expressions> left, int minPrecedence);

    //Utility functions to create AST
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token advance();
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    void consume(TokenType type, const std::string& errorMsg);
    int getPrecedence(const Token& tok);

public:
    std::vector<std::unique_ptr<Statements>> parse();
    explicit Parser(const std::vector<Token>& tokens);
    
};

#endif //PARSER_H