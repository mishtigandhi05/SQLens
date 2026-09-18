#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <string>
#include "ast.h"
#include "symbol_table.h"

/**
 * @brief Semantic Analyzer for MiniSQL.
 * 
 * Validates:
 * 1. Table existence against schema.
 * 2. Column existence in SELECT, WHERE, and ORDER BY clauses.
 * 3. Type compatibility for comparison operations (=, !=, <, >, <=, >=).
 * 4. Logical operand compatibility for AND and OR operators.
 */
class SemanticAnalyzer {
private:
    const SymbolTable& symbolTable;
    std::string currentTable;
    std::string errorMessage;

    // Recursively infers the type of an expression and checks semantic validity
    std::string inferAndCheckExpr(const ExprNode* expr);

    // Checks whether two operand data types are compatible for a given operator
    bool areTypesCompatible(const std::string& type1, const std::string& type2, const std::string& op) const;

public:
    explicit SemanticAnalyzer(const SymbolTable& symTable);

    // Analyzes a parsed SELECT query AST. Returns true if valid, false on error.
    bool analyze(const SelectQueryNode* query);

    // Returns the descriptive error message if analyze() returned false.
    const std::string& getErrorMessage() const;
};

#endif // SEMANTIC_ANALYZER_H
