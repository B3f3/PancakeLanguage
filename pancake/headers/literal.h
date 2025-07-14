#ifndef LITERAL_H
#define LITERAL_H

#include <string>
#include <vector>
#include <memory>
#include "statements.h"
#include "expressions.h"

class Literal : public Expressions {
public:
    std::string value;
    std::string type; // "int", "string", "bool", "double"

    Literal(const std::string& val, const std::string& type)
        : value(val), type(type) {}

    std::string getType() const { return type; }

    void debugPrint(int indent = 0) const override {
        std::string ind(indent, ' ');
        std::cout << ind << "Literal(" << type << "): " << value << "\n";
    }
};


#endif //LITERAL_H