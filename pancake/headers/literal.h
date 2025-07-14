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
        explicit Literal(const std::string& val) : value(val) {}

        void debugPrint(int indent = 0) const override {
            std::cout << std::string(indent, ' ') << "Literal(" << value << ")\n";
        }
};


#endif //LITERAL_H