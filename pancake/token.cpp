#include "token.h"
#include <string>
#include <variant>
#include <vector>
#include <iostream>


Token::Token(TokenType t, const std::string& v, int l, int c)
    : type(t), value(v), line(l), column(c) {}

Token::~Token(){}

void Token::debug() const {
        std::cout << "Token(" << static_cast<int>(type)
        << ", \"" << value
        << "\", line " << line
        << ", col " << column << ")\n";
}
