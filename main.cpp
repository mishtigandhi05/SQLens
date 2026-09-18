#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "ast.h"
#include "parser.tab.hpp"
#include "symbol_table.h"
#include "semantic_analyzer.h"

// Flex and Bison external symbols
extern FILE* yyin;
extern int yylineno;
extern bool g_lexical_error;
extern std::string g_lexical_error_msg;
extern std::vector<std::string> g_recorded_tokens;
extern bool g_syntax_error;
extern std::string g_syntax_error_msg;

// Lexer string input helpers implemented in lexer.l
void set_lexer_input_string(const char* str);
void clean_lexer_input();
void clear_lexer_state();

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Enable UTF-8 encoding in Windows terminal
    system("chcp 65001 > nul");
#endif

    std::string defaultQuery = "SELECT name, cgpa FROM students WHERE cgpa > 8.0;";
    std::string queryToRun;

    if (argc > 1) {
        std::string arg = argv[1];
        // Check if argument is an existing readable file
        std::ifstream file(arg);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            queryToRun = buffer.str();
            file.close();
        } else {
            // Treat as direct SQL query string
            queryToRun = arg;
        }
    } else {
        queryToRun = defaultQuery;
    }

    // Trim trailing newlines and whitespace for clean banner presentation
    while (!queryToRun.empty() && (queryToRun.back() == '\r' || queryToRun.back() == '\n' || queryToRun.back() == ' ')) {
        queryToRun.pop_back();
    }

    // Header Banner
    std::cout << "========================================\n";
    std::cout << "              SQLens COMPILER           \n";
    std::cout << "========================================\n\n";
    std::cout << "SOURCE QUERY:\n" << queryToRun << "\n\n";

    // Initialize lexer state and scan query string
    clear_lexer_state();
    set_lexer_input_string(queryToRun.c_str());
    yylineno = 1;
    g_syntax_error = false;
    g_syntax_error_msg.clear();

    // Run Bison parser (which calls Flex yylex() to tokenize and parse)
    int parseResult = yyparse();
    clean_lexer_input();

    // ----------------------------------------
    // 1. LEXICAL ANALYSIS
    // ----------------------------------------
    std::cout << "----------------------------------------\n";
    std::cout << "1. LEXICAL ANALYSIS\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << "Tokens:\n";
    for (const auto& tok : g_recorded_tokens) {
        std::cout << tok << "\n";
    }

    // Halt if a lexical error was detected
    if (g_lexical_error) {
        std::cout << "\nLexical Error:\n" << g_lexical_error_msg << "\n\n";
        std::cout << "Compilation stopped.\n";
        delete g_root_ast;
        g_root_ast = nullptr;
        return 1;
    }

    // ----------------------------------------
    // 2. SYNTAX ANALYSIS
    // ----------------------------------------
    std::cout << "\n----------------------------------------\n";
    std::cout << "2. SYNTAX ANALYSIS\n";
    std::cout << "----------------------------------------\n\n";

    // Halt if a syntax error was detected
    if (parseResult != 0 || g_syntax_error) {
        std::cout << "Syntax Error:\n" << g_syntax_error_msg << "\n\n";
        std::cout << "Compilation stopped.\n";
        delete g_root_ast;
        g_root_ast = nullptr;
        return 2;
    }

    std::cout << "Syntax analysis successful.\n";

    // ----------------------------------------
    // 3. SYMBOL TABLE
    // ----------------------------------------
    std::cout << "\n----------------------------------------\n";
    std::cout << "3. SYMBOL TABLE\n";
    std::cout << "----------------------------------------\n\n";

    // Display the schema symbol table before semantic verification
    SymbolTable symbolTable;
    symbolTable.printSymbolTable();

    // ----------------------------------------
    // 4. SEMANTIC ANALYSIS
    // ----------------------------------------
    std::cout << "\n----------------------------------------\n";
    std::cout << "4. SEMANTIC ANALYSIS\n";
    std::cout << "----------------------------------------\n\n";

    SemanticAnalyzer analyzer(symbolTable);

    // Halt if semantic verification fails
    if (!analyzer.analyze(g_root_ast)) {
        std::cout << "\nERROR:\n" << analyzer.getErrorMessage() << "\n\n";
        std::cout << "Compilation stopped.\n";
        delete g_root_ast;
        g_root_ast = nullptr;
        return 3;
    }

    std::cout << "\nSemantic analysis successful.\n";

    // ----------------------------------------
    // 5. ABSTRACT SYNTAX TREE
    // ----------------------------------------
    std::cout << "\n----------------------------------------\n";
    std::cout << "5. ABSTRACT SYNTAX TREE\n";
    std::cout << "----------------------------------------\n\n";

    if (g_root_ast) {
        g_root_ast->print();
        delete g_root_ast;
        g_root_ast = nullptr;
    } else {
        std::cout << "(Empty AST)\n";
    }

    std::cout << "\n========================================\n";

    return 0;
}

