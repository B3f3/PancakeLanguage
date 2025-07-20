#ifndef STATEMENT_H
#define STATEMENT_H

#include <string>
#include <vector>
#include "astnodes.h"

class Statements : public ASTNodes {        
    public:
        int line = 0;
        int column = 0;
        virtual ~Statements() = default;
        virtual void debugPrint(int indent = 0) const = 0;  // ← Add this
};


#endif //STATEMENTS_H