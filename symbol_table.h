#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>

/**
 * ============================================================================
 * EDUCATIONAL COMPILER PHASE: SYMBOL TABLE
 * ============================================================================
 * 1. What is a Symbol Table?
 *    A symbol table is a core data structure used by compilers to store
 *    information about program identifiers and their declared attributes.
 *    In an SQL database compiler like SQLens, symbols represent database
 *    schemas: table names, column names, data types (INT, FLOAT, STRING),
 *    and relational metadata.
 *
 * 2. Why is it needed in a Compiler?
 *    Syntax analysis (the parser) only verifies that an input query conforms to
 *    the formal grammar rules of SQL (e.g., "SELECT <columns> FROM <table>").
 *    However, the parser has no knowledge of whether the referenced table or
 *    columns actually exist, or whether their data types make sense in
 *    expressions. The symbol table bridges this gap by persisting the schema
 *    definitions so the compiler can perform semantic validation and type
 *    checking.
 *
 * 3. How SQLens uses it during Semantic Analysis:
 *    During semantic analysis, SQLens consults the SymbolTable to:
 *    - Verify table existence (e.g., hasTable("students")).
 *    - Verify column existence in the SELECT list (e.g., hasColumn("students", "name")).
 *    - Check and infer types in WHERE clause conditions and expressions
 *      (e.g., getColumnType("students", "cgpa") -> "FLOAT").
 *    - Verify column existence in ORDER BY clauses.
 *    - Ensure type safety (e.g., preventing comparing a FLOAT column with a STRING).
 * ============================================================================
 */
class SymbolTable {
private:
    // Schema storage: tableName -> (columnName -> dataType)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> schema;

    // Deterministic ordering for tables and columns for predictable display
    std::vector<std::string> tableOrder;
    std::unordered_map<std::string, std::vector<std::string>> columnOrder;

    // Helper for case-insensitive string matching
    static std::string toLower(const std::string& str);

public:
    SymbolTable();

    // Check whether a table exists in the schema
    bool hasTable(const std::string& tableName) const;

    // Check whether a column exists in a given table
    bool hasColumn(const std::string& tableName, const std::string& colName) const;

    // Retrieve the data type ("INT", "FLOAT", "STRING") of a column
    std::string getColumnType(const std::string& tableName, const std::string& colName) const;

    // Displays the schema contents cleanly in tabular format for educational inspection
    void printSymbolTable() const;

    // Convenience alias matching AST coding style
    void print() const { printSymbolTable(); }

    // Returns JSON representation of the schema for the web visualizer
    std::string toJson() const;
};

#endif // SYMBOL_TABLE_H
