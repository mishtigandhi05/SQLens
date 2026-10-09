#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include "ir.h"
#include "symbol_table.h"

/**
 * ============================================================================
 * EDUCATIONAL COMPILER PHASE: QUERY EXECUTION & RESULT VISUALIZATION
 * ============================================================================
 * The execution engine is the runtime backend of the SQL compiler.
 * It takes the optimized relational IR plan, loads table records directly
 * from local storage (students.csv), and applies relational operators in
 * dataflow order:
 * 
 * IR Plan
 *   ↓
 * SCAN (Load CSV)
 *   ↓
 * FILTER (Evaluate WHERE expression)
 *   ↓
 * SORT (Order tuples)
 *   ↓
 * PROJECT (Select attributes)
 *   ↓
 * LIMIT (Slice tuple count)
 *   ↓
 * Tabular Query Result
 * ============================================================================
 */

enum class DataType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    NULL_VAL
};

/**
 * @brief Generic typed runtime value for database attributes.
 */
struct Value {
    DataType type = DataType::NULL_VAL;
    int intVal = 0;
    double floatVal = 0.0;
    std::string strVal = "";
    bool boolVal = false;

    static Value makeInt(int v);
    static Value makeFloat(double v);
    static Value makeString(const std::string& v);
    static Value makeBool(bool v);
    static Value makeNull();

    bool isNumeric() const { return type == DataType::INT || type == DataType::FLOAT; }
    double asDouble() const;
    std::string toString() const;

    // Type-safe comparison operators
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;
};

/**
 * @brief A single database row / tuple.
 */
struct Row {
    // Map from lowercase column name to Value
    std::unordered_map<std::string, Value> fields;

    bool hasField(const std::string& col) const;
    Value getField(const std::string& col) const;
    void setField(const std::string& col, const Value& val);
};

/**
 * @brief An in-memory relational table containing schema metadata and rows.
 */
struct Table {
    std::string name;
    std::vector<std::string> columnOrder;
    std::vector<Row> rows;

    bool hasColumn(const std::string& col) const;
};

/**
 * @brief Query execution outcome and runtime trace.
 */
struct ExecutionResult {
    bool success = false;
    std::string errorMessage;
    Table resultTable;
    std::vector<std::string> executionTrace;

    void printTrace() const;
    void printTable() const;
    std::string toJson() const;
};

/**
 * @brief Educational execution engine for SQLens.
 */
class QueryExecutor {
public:
    // Executes the optimized relational plan against CSV storage
    static ExecutionResult execute(const IRNode* plan, const SymbolTable& symTable);

    // Loads a table from a CSV file (e.g. students.csv)
    static bool loadCSV(const std::string& tableName, const SymbolTable& symTable, Table& outTable, std::string& errMsg);

    // Evaluates an AST expression on a single row
    static Value evaluateExpr(const ExprNode* expr, const Row& row, std::string& errMsg);

private:
    // Executes SCAN and optional FILTER with streaming safe early stopping
    static Table executeScanAndFilter(const ScanNode* scanNode,
                                      const ExprNode* condition,
                                      const SymbolTable& symTable,
                                      int earlyLimit,
                                      size_t& outScannedRows,
                                      ExecutionResult& result);

    static Table executeNode(const IRNode* node, const SymbolTable& symTable, ExecutionResult& result, int earlyLimit = -1);
};

#endif // EXECUTOR_H
