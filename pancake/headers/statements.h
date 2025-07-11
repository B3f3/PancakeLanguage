#ifndef STATEMENT_H
#define STATEMENT_H

#include <string>
#include <vector>
#include "astnodes.h"

class Statements : public ASTNodes {        
    public:
        virtual ~Statements() = default;
};


#endif //STATEMENTS_H