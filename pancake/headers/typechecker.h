#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include <unordered_map>
#include <string>

class TypeChecker {
public:
    std::unordered_map<std::string, std::string> variableTypes;

    void declare(const std::string& name, const std::string& type) {
        variableTypes[name] = type;
    }

    std::string getType(const std::string& name) const {
        auto it = variableTypes.find(name);
        if (it != variableTypes.end()) return it->second;
        return ""; // unknown variable
    }
};

#endif