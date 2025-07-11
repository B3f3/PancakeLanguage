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
};


#endif //LITERAL_H