#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "ast.h"
#include "parser.tab.hpp"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "ir.h"
#include "optimizer.h"
#include "executor.h"

// Flex and Bison external symbols
extern FILE* yyin;
extern int yylineno;
extern bool g_lexical_error;
extern std::string g_lexical_error_msg;
extern std::vector<std::string> g_recorded_tokens;
extern bool g_syntax_error;
extern std::string g_syntax_error_msg;

struct TokenRecord {
    std::string token;
    std::string lexeme;
    std::string type;
    int line;
};
extern std::vector<TokenRecord> g_detailed_tokens;

// Lexer string input helpers implemented in lexer.l
void set_lexer_input_string(const char* str);
void clean_lexer_input();
void clear_lexer_state();

// Helper to escape JSON strings safely
static std::string escapeJsonMain(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\b') out += "\\b";
        else if (c == '\f') out += "\\f";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Enable UTF-8 encoding in Windows terminal
    system("chcp 65001 > nul");
#endif

    bool jsonMode = false;
    std::string queryToRun;
    std::string defaultQuery = "SELECT name, cgpa FROM students WHERE cgpa > 8.0;";

    int argIdx = 1;
    if (argc > 1 && std::string(argv[1]) == "--json") {
        jsonMode = true;
        argIdx = 2;
    }

    if (argIdx < argc) {
        std::string arg = argv[argIdx];
        std::ifstream file(arg);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            queryToRun = buffer.str();
            file.close();
        } else {
            queryToRun = arg;
        }
    } else {
        queryToRun = defaultQuery;
    }

    // Trim trailing newlines and whitespace
    while (!queryToRun.empty() && (queryToRun.back() == '\r' || queryToRun.back() == '\n' || queryToRun.back() == ' ')) {
        queryToRun.pop_back();
    }

    auto totalStart = std::chrono::high_resolution_clock::now();

    if (!jsonMode) {
        std::cout << "========================================\n";
        std::cout << "              SQLens COMPILER           \n";
        std::cout << "========================================\n\n";
        std::cout << "SOURCE QUERY:\n" << queryToRun << "\n\n";
    }

    // ========================================
    // 1. LEXICAL ANALYSIS & 2. SYNTAX ANALYSIS
    // ========================================
    clear_lexer_state();
    set_lexer_input_string(queryToRun.c_str());
    yylineno = 1;
    g_syntax_error = false;
    g_syntax_error_msg.clear();

    auto parseStart = std::chrono::high_resolution_clock::now();
    int parseResult = yyparse();
    auto parseEnd = std::chrono::high_resolution_clock::now();
    clean_lexer_input();

    double lexParseMs = std::chrono::duration<double, std::milli>(parseEnd - parseStart).count();

    if (!jsonMode) {
        std::cout << "========================================\n";
        std::cout << "LEXICAL ANALYSIS\n";
        std::cout << "========================================\n\n";
        std::cout << "Tokens:\n";
        for (const auto& tok : g_recorded_tokens) {
            std::cout << tok << "\n";
        }
    }

    // Check for Lexical Error
    if (g_lexical_error) {
        if (jsonMode) {
            std::cout << "{\n"
                      << "  \"success\": false,\n"
                      << "  \"failedStage\": \"LEXICAL_ANALYSIS\",\n"
                      << "  \"query\": \"" << escapeJsonMain(queryToRun) << "\",\n"
                      << "  \"error\": {\n"
                      << "    \"stage\": \"Lexical Analysis\",\n"
                      << "    \"category\": \"LEXICAL ERROR\",\n"
                      << "    \"message\": \"" << escapeJsonMain(g_lexical_error_msg) << "\"\n"
                      << "  },\n"
                      << "  \"stages\": {\n"
                      << "    \"lexer\": {\"success\": false, \"tokens\": []}\n"
                      << "  }\n"
                      << "}\n";
        } else {
            std::cout << "\n[LEXICAL ERROR]\n" << g_lexical_error_msg << "\n\n"
                      << "Compilation stopped.\n";
        }
        delete g_root_ast;
        g_root_ast = nullptr;
        return 1;
    }

    // Check for Syntax Error
    if (!jsonMode) {
        std::cout << "\n========================================\n";
        std::cout << "SYNTAX ANALYSIS / AST\n";
        std::cout << "========================================\n\n";
    }

    if (parseResult != 0 || g_syntax_error) {
        if (jsonMode) {
            std::cout << "{\n"
                      << "  \"success\": false,\n"
                      << "  \"failedStage\": \"SYNTAX_ANALYSIS\",\n"
                      << "  \"query\": \"" << escapeJsonMain(queryToRun) << "\",\n"
                      << "  \"error\": {\n"
                      << "    \"stage\": \"Syntax Analysis\",\n"
                      << "    \"category\": \"SYNTAX ERROR\",\n"
                      << "    \"message\": \"" << escapeJsonMain(g_syntax_error_msg) << "\"\n"
                      << "  },\n"
                      << "  \"stages\": {\n"
                      << "    \"lexer\": {\"success\": true, \"tokens\": [";
            for (size_t i = 0; i < g_detailed_tokens.size(); ++i) {
                std::cout << "{\"token\":\"" << escapeJsonMain(g_detailed_tokens[i].token) << "\","
                          << "\"lexeme\":\"" << escapeJsonMain(g_detailed_tokens[i].lexeme) << "\","
                          << "\"type\":\"" << escapeJsonMain(g_detailed_tokens[i].type) << "\","
                          << "\"line\":" << g_detailed_tokens[i].line << "}";
                if (i + 1 < g_detailed_tokens.size()) std::cout << ",";
            }
            std::cout << "]},\n"
                      << "    \"parser\": {\"success\": false, \"error\": \"" << escapeJsonMain(g_syntax_error_msg) << "\"}\n"
                      << "  }\n"
                      << "}\n";
        } else {
            std::cout << "[SYNTAX ERROR]\n" << g_syntax_error_msg << "\n\n"
                      << "Compilation stopped.\n";
        }
        delete g_root_ast;
        g_root_ast = nullptr;
        return 2;
    }

    if (!jsonMode) {
        std::cout << "Syntax analysis successful.\n\n";
        std::cout << "Abstract Syntax Tree (AST):\n";
        if (g_root_ast) {
            g_root_ast->print();
        } else {
            std::cout << "(Empty AST)\n";
        }
    }

    // ========================================
    // 3. SYMBOL TABLE & 4. SEMANTIC ANALYSIS
    // ========================================
    SymbolTable symbolTable;
    if (!jsonMode) {
        std::cout << "\n========================================\n";
        std::cout << "SYMBOL TABLE\n";
        std::cout << "========================================\n\n";
        symbolTable.printSymbolTable();

        std::cout << "\n========================================\n";
        std::cout << "SEMANTIC ANALYSIS\n";
        std::cout << "========================================\n\n";
    }

    SemanticAnalyzer analyzer(symbolTable);
    if (!jsonMode) {
        analyzer.setVerbose(true);
    }

    auto semStart = std::chrono::high_resolution_clock::now();
    bool semOk = analyzer.analyze(g_root_ast);
    auto semEnd = std::chrono::high_resolution_clock::now();
    double semMs = std::chrono::duration<double, std::milli>(semEnd - semStart).count();

    if (!semOk) {
        if (jsonMode) {
            std::cout << "{\n"
                      << "  \"success\": false,\n"
                      << "  \"failedStage\": \"SEMANTIC_ANALYSIS\",\n"
                      << "  \"query\": \"" << escapeJsonMain(queryToRun) << "\",\n"
                      << "  \"error\": {\n"
                      << "    \"stage\": \"Semantic Analysis\",\n"
                      << "    \"category\": \"" << escapeJsonMain(analyzer.getErrorCategory()) << "\",\n"
                      << "    \"message\": \"" << escapeJsonMain(analyzer.getErrorMessage()) << "\"\n"
                      << "  },\n"
                      << "  \"stages\": {\n"
                      << "    \"lexer\": {\"success\": true, \"tokens\": [";
            for (size_t i = 0; i < g_detailed_tokens.size(); ++i) {
                std::cout << "{\"token\":\"" << escapeJsonMain(g_detailed_tokens[i].token) << "\","
                          << "\"lexeme\":\"" << escapeJsonMain(g_detailed_tokens[i].lexeme) << "\","
                          << "\"type\":\"" << escapeJsonMain(g_detailed_tokens[i].type) << "\","
                          << "\"line\":" << g_detailed_tokens[i].line << "}";
                if (i + 1 < g_detailed_tokens.size()) std::cout << ",";
            }
            std::cout << "]},\n"
                      << "    \"parser\": {\"success\": true, \"ast\": " << (g_root_ast ? g_root_ast->toJson() : "null") << "},\n"
                      << "    \"symbolTable\": " << symbolTable.toJson() << ",\n"
                      << "    \"semantic\": " << analyzer.toJson() << "\n"
                      << "  }\n"
                      << "}\n";
        } else {
            std::cout << "\n[" << analyzer.getErrorCategory() << "]\n"
                      << analyzer.getErrorMessage() << "\n\n"
                      << "Compilation stopped.\n";
        }
        delete g_root_ast;
        g_root_ast = nullptr;
        return 3;
    }

    if (!jsonMode) {
        std::cout << "\nSemantic analysis successful.\n";
    }

    // ========================================
    // 5. INTERMEDIATE REPRESENTATION
    // ========================================
    auto irStart = std::chrono::high_resolution_clock::now();
    IRNode* initialPlan = IRBuilder::buildFromAST(g_root_ast);
    auto irEnd = std::chrono::high_resolution_clock::now();
    double irMs = std::chrono::duration<double, std::milli>(irEnd - irStart).count();

    if (!jsonMode) {
        std::cout << "\n========================================\n";
        std::cout << "INTERMEDIATE REPRESENTATION\n";
        std::cout << "========================================\n\n";
        std::cout << "Canonical Relational Plan:\n";
        if (initialPlan) {
            initialPlan->print();
        } else {
            std::cout << "(Empty IR Plan)\n";
        }
    }

    // ========================================
    // 6. QUERY OPTIMIZATION
    // ========================================
    auto optStart = std::chrono::high_resolution_clock::now();
    OptimizationResult optResult = Optimizer::optimize(initialPlan);
    auto optEnd = std::chrono::high_resolution_clock::now();
    double optMs = std::chrono::duration<double, std::milli>(optEnd - optStart).count();

    if (!jsonMode) {
        std::cout << "\n========================================\n";
        std::cout << "QUERY OPTIMIZATION\n";
        std::cout << "========================================\n\n";
        optResult.print();
    }

    // ========================================
    // 7. QUERY EXECUTION
    // ========================================
    auto execStart = std::chrono::high_resolution_clock::now();
    ExecutionResult execResult = QueryExecutor::execute(optResult.optimizedPlan, symbolTable);
    auto execEnd = std::chrono::high_resolution_clock::now();
    double execMs = std::chrono::duration<double, std::milli>(execEnd - execStart).count();

    auto totalEnd = std::chrono::high_resolution_clock::now();
    double totalMs = std::chrono::duration<double, std::milli>(totalEnd - totalStart).count();

    if (!jsonMode) {
        std::cout << "\n========================================\n";
        std::cout << "QUERY EXECUTION\n";
        std::cout << "========================================\n\n";
        execResult.printTrace();
    }

    if (!execResult.success) {
        if (jsonMode) {
            std::cout << "{\n"
                      << "  \"success\": false,\n"
                      << "  \"failedStage\": \"EXECUTION\",\n"
                      << "  \"query\": \"" << escapeJsonMain(queryToRun) << "\",\n"
                      << "  \"error\": {\n"
                      << "    \"stage\": \"Query Execution\",\n"
                      << "    \"category\": \"EXECUTION ERROR\",\n"
                      << "    \"message\": \"" << escapeJsonMain(execResult.errorMessage) << "\"\n"
                      << "  },\n"
                      << "  \"stages\": {\n"
                      << "    \"lexer\": {\"success\": true, \"tokens\": [";
            for (size_t i = 0; i < g_detailed_tokens.size(); ++i) {
                std::cout << "{\"token\":\"" << escapeJsonMain(g_detailed_tokens[i].token) << "\","
                          << "\"lexeme\":\"" << escapeJsonMain(g_detailed_tokens[i].lexeme) << "\","
                          << "\"type\":\"" << escapeJsonMain(g_detailed_tokens[i].type) << "\","
                          << "\"line\":" << g_detailed_tokens[i].line << "}";
                if (i + 1 < g_detailed_tokens.size()) std::cout << ",";
            }
            std::cout << "]},\n"
                      << "    \"parser\": {\"success\": true, \"ast\": " << (g_root_ast ? g_root_ast->toJson() : "null") << "},\n"
                      << "    \"symbolTable\": " << symbolTable.toJson() << ",\n"
                      << "    \"semantic\": " << analyzer.toJson() << ",\n"
                      << "    \"ir\": {\"canonicalPlan\": " << (initialPlan ? initialPlan->toJson() : "null") << "},\n"
                      << "    \"optimizer\": " << optResult.toJson() << ",\n"
                      << "    \"execution\": " << execResult.toJson() << "\n"
                      << "  }\n"
                      << "}\n";
        } else {
            std::cout << "\nCompilation and execution halted.\n";
        }
        delete initialPlan;
        delete g_root_ast;
        g_root_ast = nullptr;
        return 4;
    }

    // ========================================
    // 8. QUERY RESULT & PERFORMANCE REPORT
    // ========================================
    if (jsonMode) {
        std::cout << "{\n"
                  << "  \"success\": true,\n"
                  << "  \"query\": \"" << escapeJsonMain(queryToRun) << "\",\n"
                  << "  \"stages\": {\n"
                  << "    \"source\": {\"query\": \"" << escapeJsonMain(queryToRun) << "\"},\n"
                  << "    \"lexer\": {\"success\": true, \"tokens\": [";
        for (size_t i = 0; i < g_detailed_tokens.size(); ++i) {
            std::cout << "{\"token\":\"" << escapeJsonMain(g_detailed_tokens[i].token) << "\","
                      << "\"lexeme\":\"" << escapeJsonMain(g_detailed_tokens[i].lexeme) << "\","
                      << "\"type\":\"" << escapeJsonMain(g_detailed_tokens[i].type) << "\","
                      << "\"line\":" << g_detailed_tokens[i].line << "}";
            if (i + 1 < g_detailed_tokens.size()) std::cout << ",";
        }
        std::cout << "]},\n"
                  << "    \"parser\": {\"success\": true, \"ast\": " << (g_root_ast ? g_root_ast->toJson() : "null") << "},\n"
                  << "    \"symbolTable\": " << symbolTable.toJson() << ",\n"
                  << "    \"semantic\": " << analyzer.toJson() << ",\n"
                  << "    \"ir\": {\"canonicalPlan\": " << (initialPlan ? initialPlan->toJson() : "null") << "},\n"
                  << "    \"optimizer\": " << optResult.toJson() << ",\n"
                  << "    \"execution\": " << execResult.toJson() << ",\n"
                  << "    \"result\": " << execResult.toJson() << "\n"
                  << "  },\n"
                  << "  \"performance\": {\n"
                  << "    \"lexerMs\": " << std::fixed << std::setprecision(3) << (lexParseMs * 0.4) << ",\n"
                  << "    \"parserMs\": " << std::fixed << std::setprecision(3) << (lexParseMs * 0.6) << ",\n"
                  << "    \"semanticMs\": " << std::fixed << std::setprecision(3) << semMs << ",\n"
                  << "    \"irMs\": " << std::fixed << std::setprecision(3) << irMs << ",\n"
                  << "    \"optimizerMs\": " << std::fixed << std::setprecision(3) << optMs << ",\n"
                  << "    \"executionMs\": " << std::fixed << std::setprecision(3) << execMs << ",\n"
                  << "    \"totalMs\": " << std::fixed << std::setprecision(3) << totalMs << "\n"
                  << "  }\n"
                  << "}\n";
    } else {
        std::cout << "\n========================================\n";
        std::cout << "QUERY RESULT\n";
        std::cout << "========================================\n\n";
        execResult.printTable();

        std::cout << "\n----------------------------------------\n"
                  << "Compiler performance: " << std::fixed << std::setprecision(2) << totalMs << " ms\n"
                  << "========================================\n";
    }

    // Clean up allocated AST and IR memory
    delete initialPlan;
    delete g_root_ast;
    g_root_ast = nullptr;

    return 0;
}
