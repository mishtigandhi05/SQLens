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
    : symbolTable(symTable), currentTable(""), errorMessage(""), errorCategory("SEMANTIC ERROR") {}

const std::string& SemanticAnalyzer::getErrorMessage() const {
    return errorMessage;
}

const std::string& SemanticAnalyzer::getErrorCategory() const {
    return errorCategory;
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

static std::string escapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

std::string SemanticAnalyzer::inferAndCheckExpr(const ExprNode* expr) {
    if (!expr) {
        return "";
    }

    // Case A: Column Reference (e.g. name, cgpa, age)
    if (const auto* colNode = dynamic_cast<const ColumnRefNode*>(expr)) {
        if (!symbolTable.hasColumn(currentTable, colNode->name)) {
            errorCategory = "SEMANTIC ERROR";
            errorMessage = "Column '" + colNode->name + "' does not exist in table '" + currentTable + "'.";
            checklist.push_back({"WHERE Column Validation", colNode->name, false, errorMessage});
            return "";
        }
        std::string colType = symbolTable.getColumnType(currentTable, colNode->name);
        checklist.push_back({"WHERE Column Validation", colNode->name, true, "Column '" + colNode->name + "' (" + colType + ") verified in schema."});
        if (verbose) std::cout << "Column '" << colNode->name << "' found in WHERE condition.\n";
        return colType;
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
                errorCategory = "TYPE ERROR";
                errorMessage = "Operands of '" + binNode->op + "' must be BOOLEAN expressions.";
                checklist.push_back({"Logical Compatibility", binNode->op, false, errorMessage});
                return "";
            }
            checklist.push_back({"Logical Compatibility", binNode->op, true, "BOOL " + binNode->op + " BOOL -> valid."});
            if (verbose) std::cout << "Type check: BOOL " << binNode->op << " BOOL -> valid.\n";
            return "BOOL";
        }

        // Comparison Operators (=, !=, <, >, <=, >=)
        if (binNode->op == "=" || binNode->op == "!=" ||
            binNode->op == "<" || binNode->op == ">" ||
            binNode->op == "<=" || binNode->op == ">=") {
            if (!areTypesCompatible(leftType, rightType, binNode->op)) {
                if (verbose) std::cout << "\nType check:\n" << leftType << " " << binNode->op << " " << rightType << "\n";
                errorCategory = "TYPE ERROR";
                errorMessage = "Cannot compare " + leftType + " with " + rightType + ".";
                checklist.push_back({"Type Compatibility", leftType + " " + binNode->op + " " + rightType, false, errorMessage});
                return "";
            }
            checklist.push_back({"Type Compatibility", leftType + " " + binNode->op + " " + rightType, true, "Type check passed: operands are compatible."});
            if (verbose) std::cout << "Type check: " << leftType << " " << binNode->op << " " << rightType << " -> valid.\n";
            return "BOOL"; // Comparison produces a boolean condition
        }

        errorCategory = "SEMANTIC ERROR";
        errorMessage = "Unsupported operator '" + binNode->op + "'.";
        checklist.push_back({"Operator Support", binNode->op, false, errorMessage});
        return "";
    }

    errorCategory = "SEMANTIC ERROR";
    errorMessage = "Unknown expression node type in AST.";
    checklist.push_back({"Expression Parsing", "Unknown Node", false, errorMessage});
    return "";
}

bool SemanticAnalyzer::analyze(const SelectQueryNode* query) {
    errorMessage.clear();
    errorCategory = "SEMANTIC ERROR";
    checklist.clear();

    if (!query) {
        errorCategory = "SEMANTIC ERROR";
        errorMessage = "Empty or null query AST.";
        checklist.push_back({"AST Integrity", "Root Node", false, errorMessage});
        return false;
    }

    // Step 1: Check table existence against Symbol Table
    if (!symbolTable.hasTable(query->tableName)) {
        errorCategory = "SEMANTIC ERROR";
        errorMessage = "Table '" + query->tableName + "' does not exist.";
        checklist.push_back({"Table Existence", query->tableName, false, errorMessage});
        return false;
    }
    checklist.push_back({"Table Existence", query->tableName, true, "Table '" + query->tableName + "' found in schema."});
    if (verbose) std::cout << "Table '" << query->tableName << "' found.\n";
    currentTable = query->tableName;

    // Step 2: Check selected columns existence against Symbol Table
    for (const auto& col : query->columns) {
        if (!symbolTable.hasColumn(currentTable, col)) {
            errorCategory = "SEMANTIC ERROR";
            errorMessage = "Column '" + col + "' does not exist in table '" + currentTable + "'.";
            checklist.push_back({"SELECT Column Validation", col, false, errorMessage});
            return false;
        }
        std::string colType = symbolTable.getColumnType(currentTable, col);
        checklist.push_back({"SELECT Column Validation", col, true, "Column '" + col + "' (" + colType + ") verified in schema."});
        if (verbose) std::cout << "Column '" << col << "' found.\n";
    }

    // Step 3: Check WHERE clause expression (if present)
    if (query->whereClause) {
        std::string condType = inferAndCheckExpr(query->whereClause);
        if (condType.empty()) {
            return false; // Error message already recorded in inferAndCheckExpr
        }
        if (condType != "BOOL") {
            errorCategory = "TYPE ERROR";
            errorMessage = "WHERE clause condition must evaluate to a BOOLEAN expression.";
            checklist.push_back({"Predicate Evaluation", "WHERE Clause", false, errorMessage});
            return false;
        }
        checklist.push_back({"Predicate Evaluation", "WHERE Clause", true, "Predicate expression evaluates to BOOLEAN."});
    }

    // Step 4: Check ORDER BY column existence (if present)
    if (!query->orderByColumn.empty()) {
        if (!symbolTable.hasColumn(currentTable, query->orderByColumn)) {
            errorCategory = "SEMANTIC ERROR";
            errorMessage = "Column '" + query->orderByColumn + "' does not exist in table '" + currentTable + "'.";
            checklist.push_back({"ORDER BY Column Validation", query->orderByColumn, false, errorMessage});
            return false;
        }
        std::string colType = symbolTable.getColumnType(currentTable, query->orderByColumn);
        checklist.push_back({"ORDER BY Column Validation", query->orderByColumn, true, "Column '" + query->orderByColumn + "' (" + colType + ") verified in schema."});
        if (verbose) std::cout << "Column '" << query->orderByColumn << "' found in ORDER BY clause.\n";
    }

    return true;
}

std::string SemanticAnalyzer::toJson() const {
    std::string json = "{\"success\":" + std::string(errorMessage.empty() ? "true" : "false");
    json += ",\"category\":\"" + escapeJson(errorCategory) + "\"";
    json += ",\"message\":\"" + escapeJson(errorMessage) + "\"";
    json += ",\"checklist\":[";
    for (size_t i = 0; i < checklist.size(); ++i) {
        json += "{\"check\":\"" + escapeJson(checklist[i].check) + "\"";
        json += ",\"entity\":\"" + escapeJson(checklist[i].entity) + "\"";
        json += ",\"passed\":" + std::string(checklist[i].passed ? "true" : "false");
        json += ",\"message\":\"" + escapeJson(checklist[i].message) + "\"}";
        if (i + 1 < checklist.size()) json += ",";
    }
    json += "]}";
    return json;
}

