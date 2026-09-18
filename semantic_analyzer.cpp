#include "semantic_analyzer.h"
#include <iostream>

/**
 * ============================================================================
 * EDUCATIONAL COMPILER PHASE: SEMANTIC ANALYSIS
 * ============================================================================
 * Semantic Analysis is the compiler phase where the abstract syntax tree (AST)
 * is verified against language semantics and the symbol table schema.
 * 
 * In SQLens:
 * 1. Scope and Entity Binding: Verifies that tables and columns exist in the
 *    SymbolTable.
 * 2. Type Checking: Infers types of column references and literals, ensuring
 *    operands are compatible for comparison and logical operations.
 * ============================================================================
 */

// Helper function to identify numeric types
static bool isNumericType(const std::string& type) {
    return (type == "INT" || type == "FLOAT");
}

SemanticAnalyzer::SemanticAnalyzer(const SymbolTable& symTable)
    : symbolTable(symTable), currentTable(""), errorMessage("") {}

const std::string& SemanticAnalyzer::getErrorMessage() const {
    return errorMessage;
}

bool SemanticAnalyzer::areTypesCompatible(const std::string& type1,
                                          const std::string& type2,
                                          const std::string& op) const {
    // 1. Numeric types (INT and FLOAT) can be compared with each other
    if (isNumericType(type1) && isNumericType(type2)) {
        return true;
    }

    // 2. Strings can only be compared with strings
    if (type1 == "STRING" && type2 == "STRING") {
        return true;
    }

    // 3. Booleans can only be tested for equality or inequality
    if (type1 == "BOOL" && type2 == "BOOL") {
        return (op == "=" || op == "!=");
    }

    // All other cross-type comparisons are incompatible
    return false;
}

std::string SemanticAnalyzer::inferAndCheckExpr(const ExprNode* expr) {
    if (!expr) {
        return "";
    }

    // Case A: Column Reference (e.g. name, cgpa, age)
    if (const auto* colNode = dynamic_cast<const ColumnRefNode*>(expr)) {
        if (!symbolTable.hasColumn(currentTable, colNode->name)) {
            errorMessage = "Column '" + colNode->name + "' does not exist in table '" + currentTable + "'.";
            return "";
        }
        std::cout << "Column '" << colNode->name << "' found in WHERE condition.\n";
        return symbolTable.getColumnType(currentTable, colNode->name);
    }

    // Case B: Literal Constant (e.g. 8.0, 20, 'Alice', TRUE)
    if (const auto* litNode = dynamic_cast<const LiteralNode*>(expr)) {
        if (litNode->type == "BOOLEAN" || litNode->type == "BOOL") {
            return "BOOL";
        }
        return litNode->type; // "INT", "FLOAT", "STRING"
    }

    // Case C: Binary Operation (AND, OR, =, !=, <, >, <=, >=)
    if (const auto* binNode = dynamic_cast<const BinaryOpNode*>(expr)) {
        // Recursively validate left operand
        std::string leftType = inferAndCheckExpr(binNode->left);
        if (leftType.empty()) {
            return ""; // Propagate error from left child
        }

        // Recursively validate right operand
        std::string rightType = inferAndCheckExpr(binNode->right);
        if (rightType.empty()) {
            return ""; // Propagate error from right child
        }

        // Logical Operators (AND, OR)
        if (binNode->op == "AND" || binNode->op == "OR") {
            if (leftType != "BOOL" || rightType != "BOOL") {
                errorMessage = "Operands of '" + binNode->op + "' must be BOOLEAN expressions.";
                return "";
            }
            std::cout << "Type check: BOOL " << binNode->op << " BOOL -> valid.\n";
            return "BOOL";
        }

        // Comparison Operators (=, !=, <, >, <=, >=)
        if (binNode->op == "=" || binNode->op == "!=" ||
            binNode->op == "<" || binNode->op == ">" ||
            binNode->op == "<=" || binNode->op == ">=") {
            if (!areTypesCompatible(leftType, rightType, binNode->op)) {
                std::cout << "\nType check:\n" << leftType << " " << binNode->op << " " << rightType << "\n";
                errorMessage = "Type mismatch: cannot compare " + leftType + " with " + rightType + ".";
                return "";
            }
            std::cout << "Type check: " << leftType << " " << binNode->op << " " << rightType << " -> valid.\n";
            return "BOOL"; // Comparison produces a boolean condition
        }

        errorMessage = "Unsupported operator '" + binNode->op + "'.";
        return "";
    }

    errorMessage = "Unknown expression node type in AST.";
    return "";
}

bool SemanticAnalyzer::analyze(const SelectQueryNode* query) {
    errorMessage.clear();

    if (!query) {
        errorMessage = "Empty or null query AST.";
        return false;
    }

    // Step 1: Check table existence against Symbol Table
    if (!symbolTable.hasTable(query->tableName)) {
        errorMessage = "Table '" + query->tableName + "' does not exist.";
        return false;
    }
    std::cout << "Table '" << query->tableName << "' found.\n";
    currentTable = query->tableName;

    // Step 2: Check selected columns existence against Symbol Table
    for (const auto& col : query->columns) {
        if (!symbolTable.hasColumn(currentTable, col)) {
            errorMessage = "Column '" + col + "' does not exist in table '" + currentTable + "'.";
            return false;
        }
        std::cout << "Column '" << col << "' found.\n";
    }

    // Step 3: Check WHERE clause expression (if present)
    if (query->whereClause) {
        std::string condType = inferAndCheckExpr(query->whereClause);
        if (condType.empty()) {
            return false; // Error message already recorded in inferAndCheckExpr
        }
        if (condType != "BOOL") {
            errorMessage = "WHERE clause condition must evaluate to a BOOLEAN expression.";
            return false;
        }
    }

    // Step 4: Check ORDER BY column existence (if present)
    if (!query->orderByColumn.empty()) {
        if (!symbolTable.hasColumn(currentTable, query->orderByColumn)) {
            errorMessage = "Column '" + query->orderByColumn + "' does not exist in table '" + currentTable + "'.";
            return false;
        }
        std::cout << "Column '" << query->orderByColumn << "' found in ORDER BY clause.\n";
    }

    return true;
}

