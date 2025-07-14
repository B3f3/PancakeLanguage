#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <string>
#include <vector>
#include "astnodes.h"

class Expressions : public ASTNodes {        
    public:
        virtual ~Expressions() = default;
        virtual void debugPrint(int indent = 0) const = 0;  // ← Add this
};


#endif //EXPRESSIONS_H