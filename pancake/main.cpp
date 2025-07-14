#include "./headers/tokeniser.h"
#include "./headers/parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Function prototypes
void runConsole();
void runFile(const std::string& filename);

int main(int argc, char* argv[]) {
    if (argc == 1) {
        runConsole();
    } else if (argc == 2) {
        // One argument → treat as filename and run script
        runFile(argv[1]);
    } else {
        std::cerr << "Usage:\n";
        std::cerr << "  " << argv[0] << "          # interactive mode\n";
        std::cerr << "  " << argv[0] << " file.pnc  # run script\n";
        return 1;
    }
    return 0;
}

void runConsole() {
    // version, compiler, and platform info
    std::string version = "Pancake 0.0.1";
    std::string buildDate = __DATE__;
    std::string buildTime = __TIME__;
    std::string compiler = 
    #if defined(_MSC_VER)
        "MSVC";
    #elif defined(__clang__)
        "Clang";
    #elif defined(__GNUC__)
        "GCC";
    #else
        "Unknown Compiler";
    #endif

    std::string platform =
    #if defined(_WIN32)
        "win32";
    #elif defined(__linux__)
        "linux";
    #elif defined(__APPLE__)
        "darwin";
    #else
        "unknown";
    #endif

    std::cout << version << " (built on " << buildDate << ", " << buildTime << ") [" << compiler << "] on " << platform << "\n";
    std::cout << "Type \"help;\" for information\n";

    std::string line;
    TypeChecker checker;
    while (true) {
        std::cout << "pan -> ";
        std::getline(std::cin, line);
        if (line == "exit;" || std::cin.eof()) break;
        if (line == "help;") {
            std::cout << "This is " << version <<" - Type code or use commands like 'exit;'.\n";
            continue;
        }

        try
        {
            // TODO: parse and evaluate `line`
            std::cout << "=== Line in ===: " << line << "\n==============\n";

            Tokeniser lexer(line);
            lexer.tokenize();  // Fills the tokens list
            
            std::cout << "=== Tokens ===\n";
            for (const Token& t : lexer.getTokens()) {
                t.debug();
            }
            std::cout << "==============\n";

            Parser parser(lexer.getTokens(), checker);
            auto ast = parser.parse();

            std::cout << "=== AST ===\n";
            for (const auto& stmt : ast) {
                stmt->debugPrint();
            }
            std::cout << "==========\n";
        }
        catch(const std::exception& e)
        {
            std::cerr << "Syntax Error: " << e.what() << '\n';
        }

    }
}

void runFile(const std::string& filename) {
    std::ifstream file(filename);
    TypeChecker checker;
    
    if (!file) {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return;
    }

    try {
        // Read entire file
        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string fullSource = buffer.str();

        std::cout << "=== Source ===\n" << fullSource << "\n==============\n";

        // Tokenize
        Tokeniser lexer(fullSource);
        lexer.tokenize();

        // Debug print tokens
        std::cout << "=== Tokens ===\n";
        for (const Token& t : lexer.getTokens()) {
            t.debug();
        }
        std::cout << "==============\n";

        // Parse
        Parser parser(lexer.getTokens(), checker);
        auto ast = parser.parse();

        // Execute (debug print)
        std::cout << "=== AST ===\n";
        for (const auto& stmt : ast) {
            stmt->debugPrint();
        }
        std::cout << "==========\n";

    } catch (const std::exception& e) {
        std::cerr << "Error in " << filename << ": " << e.what() << '\n';
    }
}
