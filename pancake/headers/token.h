#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <variant>
#include <vector>
#include <iostream>

    // All possible token types
    enum class TokenType {
        // Keywords
        LET,
        OUT,
        IN,
        IF,
        ELIF,
        ELSE,
        MOD,
        
        // Types
        TYPE_INT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_BOOL,

        // Operators
        PLUS,
        MINUS,
        MUL,
        DIV,
        MODULO,
        EQ,         // =
        EE,         // ==
        NE,         // !=
        LT,         // <
        GT,         // >
        LTE,        // <=
        GTE,        // >=
        ARROWF,     // ->
        ARROWB,     // <-
        AND,
        OR,
        NOT,

        // Punctuation
        LPAREN,     // (
        RPAREN,     // )
        LBRACE,     // {
        RBRACE,     // }
        SEMICOLON,
        COMMA,

        // Literals and identifiers
        IDENTIFIER,
        INT_LITERAL,
        DOUBLE_LITERAL,
        STRING_LITERAL,
        BOOL_LITERAL,

        // out ENDL and Backslas_n, End-of-line, End-of-file or invalid
        ENDL,
        BACKSLAH_N,
        END_OF_LINE,
        END_OF_FILE,
        UNKNOWN
    };

    // A single token
    class Token {
        public:
            TokenType type;
            std::string value;
            int line;
            int column;
            
            Token(TokenType t, const std::string& v, int l, int c);
            ~Token();

            void debug() const ;

    };
#endif // TOKEN_H
