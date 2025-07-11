#ifndef TOKENISER_H
#define TOKENISER_H

#include <string>
#include <vector>
#include "token.h"

class Tokeniser {
    private:
        std::vector<Token> tokens;
        std::size_t tokenIndex = 0;

        std::string source;
        std::size_t pos;
        int line;
        int col;

        char currentChar();
        char peek() const;
        void advance();
        void skipWhitespace();
        void skipComment();
        bool isAtEnd() const;

        Token makeNumber();
        Token makeString();
        Token makeIdentifierOrKeyword();
        Token nextToken();  // internal use

    public:
        explicit Tokeniser(const std::string& source);
        ~Tokeniser() = default;

        void tokenize();               // fill the `tokens` vector
        Token token();                 // return current token and advance
        const std::vector<Token>& getTokens() const;  // optional getter

};

#endif // TOKENISER_H