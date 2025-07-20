#include "./headers/tokeniser.h"
#include <cctype>
#include <iostream>

Tokeniser::Tokeniser(const std::string& src)
    : source(src), pos(0), line(1), col(1), tokenIndex(0) {}

char Tokeniser::currentChar() {
    return isAtEnd() ? '\0' : source[pos];
}

char Tokeniser::peek() const {
    return (pos + 1 < source.length()) ? source[pos + 1] : '\0';
}

void Tokeniser::advance() {
    if (currentChar() == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    pos++;
}

bool Tokeniser::isAtEnd() const {
    return pos >= source.length();
}

void Tokeniser::skipWhitespace() {
    while (!isAtEnd() && currentChar() != '\n' && std::isspace(static_cast<unsigned char>(currentChar()))) advance();
}

void Tokeniser::skipComment() {
    if (currentChar() == '/' && peek() == '/') {
        while (!isAtEnd() && currentChar() != '\n') advance();
    }
}

Token Tokeniser::makeNumber() {
    int startCol = col;
    std::size_t start = pos;
    bool isFloat = false;

    while (isdigit(currentChar())) advance();

    if (currentChar() == '.' && isdigit(peek())) {
        isFloat = true;
        advance();
        while (isdigit(currentChar())) advance();
    }

    std::string value = source.substr(start, pos - start);
    TokenType type = isFloat ? TokenType::DOUBLE_LITERAL : TokenType::INT_LITERAL;
    return Token(type, value, line, startCol);
}

Token Tokeniser::makeString() {
    int startCol = col;
    advance();
    std::size_t start = pos;
    while (!isAtEnd() && currentChar() != '"') advance();
    std::string value = source.substr(start, pos - start);
    advance();
    return Token(TokenType::STRING_LITERAL, value, line, startCol);
}

Token Tokeniser::makeIdentifierOrKeyword() {
    std::size_t start = pos;
    int startCol = col;
    while (isalnum(currentChar()) || currentChar() == '_') advance();
    std::string value = source.substr(start, pos - start);

    // keywords/types/booleans
    if (value == "let") return Token(TokenType::LET, value, line, startCol);
    if (value == "out") return Token(TokenType::OUT, value, line, startCol);
    if (value == "in") return Token(TokenType::IN, value, line, startCol);
    if (value == "if") return Token(TokenType::IF, value, line, startCol);
    if (value == "elif") return Token(TokenType::ELIF, value, line, startCol);
    if (value == "else") return Token(TokenType::ELSE, value, line, startCol);
    if (value == "mod") return Token(TokenType::MODULO, value, line, startCol);
    if (value == "int") return Token(TokenType::TYPE_INT, value, line, startCol);
    if (value == "double") return Token(TokenType::TYPE_DOUBLE, value, line, startCol);
    if (value == "string") return Token(TokenType::TYPE_STRING, value, line, startCol);
    if (value == "bool") return Token(TokenType::TYPE_BOOL, value, line, startCol);
    if (value == "true" || value == "false") return Token(TokenType::BOOL_LITERAL, value, line, startCol);

    if (value == "and") return Token(TokenType::AND, value, line, startCol);
    if (value == "or") return Token(TokenType::OR, value, line, startCol);

    return Token(TokenType::IDENTIFIER, value, line, startCol);
}

Token Tokeniser::nextToken() {
    skipWhitespace();
    skipComment();
    skipWhitespace();

    if (isAtEnd()) return Token(TokenType::END_OF_FILE, "", line, col);

    int startCol = col;
    char c = currentChar();

    if (isdigit(c)) return makeNumber();
    if (isalpha(c) || c == '_') return makeIdentifierOrKeyword();
    if (c == '"') return makeString();
    if (c == '\n') {
        advance();
        return Token(TokenType::END_OF_LINE, "end_of_line", line - 1, startCol);
    }

    Token token(TokenType::UNKNOWN, std::string(1, c), line, col);
    switch (c) {
        case '+': token = Token(TokenType::PLUS, "+", line, col); break;
        case '-':
            if (peek() == '>') { advance(); token = Token(TokenType::ARROWF, "->", line, col); }
            else token = Token(TokenType::MINUS, "-", line, col); break; 
        case '*': token = Token(TokenType::MUL, "*", line, col); break;
        case '/': token = Token(TokenType::DIV, "/", line, col); break;
        case '=':
            if (peek() == '=') { advance(); token = Token(TokenType::EE, "==", line, col); }
            else token = Token(TokenType::EQ, "=", line, col); break;
        case '!':
            if (peek() == '=') { advance(); token = Token(TokenType::NE, "!=", line, col); }
            else token = Token(TokenType::NOT, "!", line, col); break;
        case '>':
            if (peek() == '=') { advance(); token = Token(TokenType::GTE, ">=", line, col); }
            else token = Token(TokenType::GT, ">", line, col); break;
        case '<':
            if (peek() == '=') { advance(); token = Token(TokenType::LTE, "<=", line, col); }
            else if (peek() == '-') { advance(); token = Token(TokenType::ARROWB, "<-", line, col); }
            else token = Token(TokenType::LT, "<", line, col); break;
        case ';': token = Token(TokenType::SEMICOLON, ";", line, col); break;
        case '(': token = Token(TokenType::LPAREN, "(", line, col); break;
        case ')': token = Token(TokenType::RPAREN, ")", line, col); break;
        case '{': token = Token(TokenType::LBRACE, "{", line, col); break;
        case '}': token = Token(TokenType::RBRACE, "}", line, col); break;
        case ',': token = Token(TokenType::COMMA, ",", line, col); break;
        default: break;
    }
    advance();
    return token;
}

void Tokeniser::tokenize() {
    tokens.clear();
    tokenIndex = 0;
    while (!isAtEnd()) {
        Token t = nextToken();
        if (t.type != TokenType::END_OF_FILE) {
            tokens.push_back(t);
        } else {
            break;
        }
    }
    tokens.emplace_back(TokenType::END_OF_FILE, "end_of_file", line, col);
}

Token Tokeniser::token() {
    if (tokenIndex < tokens.size()) {
        return tokens[tokenIndex++];
    }
    return Token(TokenType::END_OF_FILE, "end_of_file", line, col);
}

const std::vector<Token>& Tokeniser::getTokens() const {
    return tokens;
}
