#include "executor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>

// ==========================================
// Helper: Case-insensitive string utility
// ==========================================
static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower;
}

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// ==========================================
// Value Implementation
// ==========================================
Value Value::makeInt(int v) {
    Value val;
    val.type = DataType::INT;
    val.intVal = v;
    return val;
}

Value Value::makeFloat(double v) {
    Value val;
    val.type = DataType::FLOAT;
    val.floatVal = v;
    return val;
}

Value Value::makeString(const std::string& v) {
    Value val;
    val.type = DataType::STRING;
    val.strVal = v;
    return val;
}

Value Value::makeBool(bool v) {
    Value val;
    val.type = DataType::BOOL;
    val.boolVal = v;
    return val;
}

Value Value::makeNull() {
    Value val;
    val.type = DataType::NULL_VAL;
    return val;
}

double Value::asDouble() const {
    if (type == DataType::INT) return static_cast<double>(intVal);
    if (type == DataType::FLOAT) return floatVal;
    return 0.0;
}

std::string Value::toString() const {
    switch (type) {
        case DataType::INT:
            return std::to_string(intVal);
        case DataType::FLOAT: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << floatVal;
            return oss.str();
        }
        case DataType::STRING:
            return strVal;
        case DataType::BOOL:
            return boolVal ? "TRUE" : "FALSE";
        case DataType::NULL_VAL:
            return "NULL";
    }
    return "";
}

bool Value::operator==(const Value& other) const {
    if (isNumeric() && other.isNumeric()) {
        return asDouble() == other.asDouble();
    }
    if (type == DataType::STRING && other.type == DataType::STRING) {
        return strVal == other.strVal;
    }
    if (type == DataType::BOOL && other.type == DataType::BOOL) {
        return boolVal == other.boolVal;
    }
    return false;
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

bool Value::operator<(const Value& other) const {
    if (isNumeric() && other.isNumeric()) {
        return asDouble() < other.asDouble();
    }
    if (type == DataType::STRING && other.type == DataType::STRING) {
        return strVal < other.strVal;
    }
    if (type == DataType::BOOL && other.type == DataType::BOOL) {
        return (!boolVal && other.boolVal);
    }
    return false;
}

bool Value::operator<=(const Value& other) const {
    return (*this < other) || (*this == other);
}

bool Value::operator>(const Value& other) const {
    return !(*this <= other);
}

bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}

// ==========================================
// Row Implementation
// ==========================================
bool Row::hasField(const std::string& col) const {
    return fields.find(toLower(col)) != fields.end();
}

Value Row::getField(const std::string& col) const {
    auto it = fields.find(toLower(col));
    if (it != fields.end()) {
        return it->second;
    }
    return Value::makeNull();
}

void Row::setField(const std::string& col, const Value& val) {
    fields[toLower(col)] = val;
}

// ==========================================
// Table Implementation
// ==========================================
bool Table::hasColumn(const std::string& col) const {
    std::string lower = toLower(col);
    for (const auto& c : columnOrder) {
        if (toLower(c) == lower) return true;
    }
    return false;
}

// ==========================================
// ExecutionResult Visualizers
// ==========================================
void ExecutionResult::printTrace() const {
    std::cout << "Execution Trace:\n";
    for (size_t i = 0; i < executionTrace.size(); ++i) {
        std::cout << (i + 1) << ". " << executionTrace[i] << "\n";
    }
    if (success) {
        std::cout << "\nQuery execution completed successfully.\n";
    } else {
        std::cout << "\n[EXECUTION ERROR]\n" << errorMessage << "\n";
    }
}

void ExecutionResult::printTable() const {
    if (!success) {
        std::cout << "No results due to execution error.\n";
        return;
    }

    if (resultTable.columnOrder.empty()) {
        std::cout << "(No columns in result table)\n";
        return;
    }

    // Determine column widths dynamically for clean table alignment
    std::vector<size_t> colWidths(resultTable.columnOrder.size());
    for (size_t i = 0; i < resultTable.columnOrder.size(); ++i) {
        colWidths[i] = resultTable.columnOrder[i].size();
        for (const auto& row : resultTable.rows) {
            std::string valStr = row.getField(resultTable.columnOrder[i]).toString();
            if (valStr.size() > colWidths[i]) {
                colWidths[i] = valStr.size();
            }
        }
        // Minimum column width for neat display
        if (colWidths[i] < 10) {
            colWidths[i] = 10;
        }
    }

    // Calculate total separator line width
    size_t totalWidth = 0;
    for (size_t w : colWidths) {
        totalWidth += w + 2; // 2 spaces padding between columns
    }
    if (totalWidth < 32) totalWidth = 32;

    std::string separator(totalWidth, '-');

    std::cout << separator << "\n";

    // Header row
    for (size_t i = 0; i < resultTable.columnOrder.size(); ++i) {
        std::cout << std::left << std::setw(colWidths[i] + 2) << resultTable.columnOrder[i];
    }
    std::cout << "\n" << separator << "\n";

    // Data rows
    for (const auto& row : resultTable.rows) {
        for (size_t i = 0; i < resultTable.columnOrder.size(); ++i) {
            std::string valStr = row.getField(resultTable.columnOrder[i]).toString();
            std::cout << std::left << std::setw(colWidths[i] + 2) << valStr;
        }
        std::cout << "\n";
    }

    std::cout << separator << "\n";
    std::cout << "Rows returned: " << resultTable.rows.size() << "\n";
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

std::string ExecutionResult::toJson() const {
    std::string json = "{";
    json += "\"success\":" + std::string(success ? "true" : "false");
    json += ",\"errorMessage\":\"" + escapeJson(errorMessage) + "\"";
    json += ",\"trace\":[";
    for (size_t i = 0; i < executionTrace.size(); ++i) {
        json += "\"" + escapeJson(executionTrace[i]) + "\"";
        if (i + 1 < executionTrace.size()) json += ",";
    }
    json += "]";
    json += ",\"columns\":[";
    for (size_t i = 0; i < resultTable.columnOrder.size(); ++i) {
        json += "\"" + escapeJson(resultTable.columnOrder[i]) + "\"";
        if (i + 1 < resultTable.columnOrder.size()) json += ",";
    }
    json += "]";
    json += ",\"rows\":[";
    for (size_t r = 0; r < resultTable.rows.size(); ++r) {
        json += "[";
        for (size_t c = 0; c < resultTable.columnOrder.size(); ++c) {
            std::string valStr = resultTable.rows[r].getField(resultTable.columnOrder[c]).toString();
            json += "\"" + escapeJson(valStr) + "\"";
            if (c + 1 < resultTable.columnOrder.size()) json += ",";
        }
        json += "]";
        if (r + 1 < resultTable.rows.size()) json += ",";
    }
    json += "]";
    json += ",\"rowCount\":" + std::to_string(resultTable.rows.size());
    json += "}";
    return json;
}

// ==========================================
// CSV File Loader
// ==========================================
bool QueryExecutor::loadCSV(const std::string& tableName,
                            const SymbolTable& symTable,
                            Table& outTable,
                            std::string& errMsg) {
    std::string fileName = tableName + ".csv";
    std::ifstream file(fileName);
    if (!file.is_open()) {
        errMsg = "Unable to load data file '" + fileName + "'.";
        return false;
    }

    outTable.name = tableName;
    outTable.columnOrder.clear();
    outTable.rows.clear();

    std::string line;
    // Read header line
    if (!std::getline(file, line) || trim(line).empty()) {
        errMsg = "CSV file '" + fileName + "' is empty.";
        return false;
    }

    std::stringstream headerStream(line);
    std::string headerToken;
    while (std::getline(headerStream, headerToken, ',')) {
        headerToken = trim(headerToken);
        if (!headerToken.empty()) {
            outTable.columnOrder.push_back(headerToken);
        }
    }

    if (outTable.columnOrder.empty()) {
        errMsg = "CSV file '" + fileName + "' does not contain valid column headers.";
        return false;
    }

    // Read record rows
    size_t lineNum = 1;
    while (std::getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty()) continue; // skip blank lines

        std::stringstream rowStream(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(rowStream, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() != outTable.columnOrder.size()) {
            errMsg = "Malformed row in CSV file '" + fileName + "' at line " +
                     std::to_string(lineNum) + ": expected " +
                     std::to_string(outTable.columnOrder.size()) + " columns, found " +
                     std::to_string(tokens.size()) + ".";
            return false;
        }

        Row row;
        for (size_t i = 0; i < outTable.columnOrder.size(); ++i) {
            const std::string& colName = outTable.columnOrder[i];
            const std::string& valStr = tokens[i];
            std::string colType = symTable.getColumnType(tableName, colName);

            try {
                if (colType == "INT") {
                    row.setField(colName, Value::makeInt(std::stoi(valStr)));
                } else if (colType == "FLOAT") {
                    row.setField(colName, Value::makeFloat(std::stod(valStr)));
                } else if (colType == "BOOL" || colType == "BOOLEAN") {
                    std::string lower = toLower(valStr);
                    row.setField(colName, Value::makeBool(lower == "true" || lower == "1"));
                } else {
                    // Default to STRING
                    row.setField(colName, Value::makeString(valStr));
                }
            } catch (const std::exception& e) {
                errMsg = "Data format error in '" + fileName + "' at line " +
                         std::to_string(lineNum) + " for column '" + colName + "': " + e.what();
                return false;
            }
        }
        outTable.rows.push_back(row);
    }

    return true;
}

// ==========================================
// Expression Evaluator on Row
// ==========================================
Value QueryExecutor::evaluateExpr(const ExprNode* expr, const Row& row, std::string& errMsg) {
    if (!expr) {
        return Value::makeBool(true);
    }

    // Column Reference
    if (const auto* colNode = dynamic_cast<const ColumnRefNode*>(expr)) {
        if (!row.hasField(colNode->name)) {
            errMsg = "Column '" + colNode->name + "' not found in table row.";
            return Value::makeNull();
        }
        return row.getField(colNode->name);
    }

    // Literal Constant
    if (const auto* litNode = dynamic_cast<const LiteralNode*>(expr)) {
        try {
            if (litNode->type == "INT") {
                return Value::makeInt(std::stoi(litNode->value));
            } else if (litNode->type == "FLOAT") {
                return Value::makeFloat(std::stod(litNode->value));
            } else if (litNode->type == "BOOLEAN" || litNode->type == "BOOL") {
                std::string lower = toLower(litNode->value);
                return Value::makeBool(lower == "true" || lower == "1");
            } else {
                return Value::makeString(litNode->value);
            }
        } catch (const std::exception& e) {
            errMsg = "Invalid literal value '" + litNode->value + "': " + e.what();
            return Value::makeNull();
        }
    }

    // Binary Operation (Comparison or Logical AND/OR)
    if (const auto* binNode = dynamic_cast<const BinaryOpNode*>(expr)) {
        Value leftVal = evaluateExpr(binNode->left, row, errMsg);
        if (!errMsg.empty()) return Value::makeNull();

        Value rightVal = evaluateExpr(binNode->right, row, errMsg);
        if (!errMsg.empty()) return Value::makeNull();

        if (binNode->op == "AND") {
            return Value::makeBool(leftVal.boolVal && rightVal.boolVal);
        } else if (binNode->op == "OR") {
            return Value::makeBool(leftVal.boolVal || rightVal.boolVal);
        } else if (binNode->op == "=") {
            return Value::makeBool(leftVal == rightVal);
        } else if (binNode->op == "!=") {
            return Value::makeBool(leftVal != rightVal);
        } else if (binNode->op == "<") {
            return Value::makeBool(leftVal < rightVal);
        } else if (binNode->op == "<=") {
            return Value::makeBool(leftVal <= rightVal);
        } else if (binNode->op == ">") {
            return Value::makeBool(leftVal > rightVal);
        } else if (binNode->op == ">=") {
            return Value::makeBool(leftVal >= rightVal);
        } else {
            errMsg = "Unsupported operator '" + binNode->op + "' in expression.";
            return Value::makeNull();
        }
    }

    errMsg = "Unknown expression node type during execution.";
    return Value::makeNull();
}

// ==========================================
// Relational Operator Tree Execution
// ==========================================

// Helper: check if operator subtree contains an operator type
static bool planHasOperator(const IRNode* node, IROpType type) {
    if (!node) return false;
    if (node->getType() == type) return true;
    return planHasOperator(node->getChild(), type);
}

// Helper: retrieve limit value from plan
static int planGetLimit(const IRNode* node) {
    if (!node) return -1;
    if (node->getType() == IROpType::LIMIT) {
        const auto* lim = dynamic_cast<const LimitNode*>(node);
        return lim ? lim->limitValue : -1;
    }
    return planGetLimit(node->getChild());
}

Table QueryExecutor::executeScanAndFilter(const ScanNode* scanNode,
                                          const ExprNode* condition,
                                          const SymbolTable& symTable,
                                          int earlyLimit,
                                          size_t& outScannedRows,
                                          ExecutionResult& result) {
    outScannedRows = 0;
    if (!scanNode) {
        result.success = false;
        result.errorMessage = "Null SCAN operator in execution plan.";
        return {};
    }

    std::string tableName = scanNode->tableName;
    std::string fileName = tableName + ".csv";
    std::ifstream file(fileName);
    if (!file.is_open()) {
        result.success = false;
        result.errorMessage = "Unable to load data file '" + fileName + "'.";
        return {};
    }

    Table table;
    table.name = tableName;

    std::string line;
    if (!std::getline(file, line) || trim(line).empty()) {
        result.success = false;
        result.errorMessage = "CSV file '" + fileName + "' is empty.";
        return {};
    }

    std::stringstream headerStream(line);
    std::string headerToken;
    while (std::getline(headerStream, headerToken, ',')) {
        headerToken = trim(headerToken);
        if (!headerToken.empty()) {
            table.columnOrder.push_back(headerToken);
        }
    }

    if (table.columnOrder.empty()) {
        result.success = false;
        result.errorMessage = "CSV file '" + fileName + "' does not contain valid column headers.";
        return {};
    }

    bool earlyStopped = false;
    size_t lineNum = 1;

    // If LIMIT 0 was requested without ORDER BY, stop before reading any data rows
    if (earlyLimit == 0) {
        earlyStopped = true;
    } else {
        while (std::getline(file, line)) {
            lineNum++;
            line = trim(line);
            if (line.empty()) continue;

            outScannedRows++;

            std::stringstream rowStream(line);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(rowStream, token, ',')) {
                tokens.push_back(trim(token));
            }

            if (tokens.size() != table.columnOrder.size()) {
                result.success = false;
                result.errorMessage = "Malformed row in CSV file '" + fileName + "' at line " +
                                      std::to_string(lineNum) + ": expected " +
                                      std::to_string(table.columnOrder.size()) + " columns, found " +
                                      std::to_string(tokens.size()) + ".";
                return {};
            }

            Row row;
            for (size_t i = 0; i < table.columnOrder.size(); ++i) {
                const std::string& colName = table.columnOrder[i];
                const std::string& valStr = tokens[i];
                std::string colType = symTable.getColumnType(tableName, colName);

                try {
                    if (colType == "INT") {
                        row.setField(colName, Value::makeInt(std::stoi(valStr)));
                    } else if (colType == "FLOAT") {
                        row.setField(colName, Value::makeFloat(std::stod(valStr)));
                    } else if (colType == "BOOL" || colType == "BOOLEAN") {
                        std::string lower = toLower(valStr);
                        row.setField(colName, Value::makeBool(lower == "true" || lower == "1"));
                    } else {
                        row.setField(colName, Value::makeString(valStr));
                    }
                } catch (const std::exception& e) {
                    result.success = false;
                    result.errorMessage = "Data format error in '" + fileName + "' at line " +
                                          std::to_string(lineNum) + " for column '" + colName + "': " + e.what();
                    return {};
                }
            }

            // If condition exists, evaluate predicate on row
            if (condition) {
                std::string evalErr;
                Value match = evaluateExpr(condition, row, evalErr);
                if (!evalErr.empty()) {
                    result.success = false;
                    result.errorMessage = evalErr;
                    return {};
                }
                if (!match.boolVal) {
                    continue; // row does not satisfy filter
                }
            }

            table.rows.push_back(row);

            // Safe Early Stopping: If row limit is satisfied and no global sorting is required,
            // stop scanning immediately without processing subsequent CSV records.
            if (earlyLimit >= 0 && table.rows.size() == static_cast<size_t>(earlyLimit)) {
                earlyStopped = true;
                break;
            }
        }
    }

    // Record SCAN step in execution trace
    std::string scanTrace = "SCAN " + tableName + " → " + std::to_string(outScannedRows) + " rows loaded";
    if (earlyStopped) {
        scanTrace += " (early stopping applied)";
    }
    result.executionTrace.push_back(scanTrace);

    // Record FILTER step in execution trace if filter condition was present
    if (condition) {
        std::string filterTrace = "FILTER " + condition->toString() + " → " +
                                  std::to_string(table.rows.size()) + " rows remain";
        if (earlyStopped) {
            filterTrace += " (early stopping applied)";
        }
        result.executionTrace.push_back(filterTrace);
    }

    return table;
}

Table QueryExecutor::executeNode(const IRNode* node, const SymbolTable& symTable, ExecutionResult& result, int earlyLimit) {
    if (!node) {
        result.success = false;
        result.errorMessage = "Null operator in execution plan.";
        return {};
    }

    // 1. LIMIT Operator
    if (const auto* limitNode = dynamic_cast<const LimitNode*>(node)) {
        if (limitNode->limitValue < 0) {
            result.success = false;
            result.errorMessage = "LIMIT value cannot be negative: " +
                                  std::to_string(limitNode->limitValue) + ".";
            return {};
        }

        Table inputTable = executeNode(limitNode->child, symTable, result, earlyLimit);
        if (!result.success) return {};

        size_t limit = static_cast<size_t>(limitNode->limitValue);
        if (inputTable.rows.size() > limit) {
            inputTable.rows.resize(limit);
        }

        result.executionTrace.push_back("LIMIT " + std::to_string(limitNode->limitValue) +
                                        " → " + std::to_string(inputTable.rows.size()) + " rows returned");
        return inputTable;
    }

    // 2. PROJECT Operator
    if (const auto* projNode = dynamic_cast<const ProjectNode*>(node)) {
        Table inputTable = executeNode(projNode->child, symTable, result, earlyLimit);
        if (!result.success) return {};

        // Verify that all projected columns exist in input table
        for (const auto& col : projNode->columns) {
            if (!inputTable.hasColumn(col)) {
                result.success = false;
                result.errorMessage = "Projected column '" + col +
                                      "' does not exist in relation.";
                return {};
            }
        }

        Table projTable;
        projTable.name = inputTable.name;
        projTable.columnOrder = projNode->columns;

        for (const auto& row : inputTable.rows) {
            Row newRow;
            for (const auto& col : projNode->columns) {
                newRow.setField(col, row.getField(col));
            }
            projTable.rows.push_back(newRow);
        }

        std::ostringstream oss;
        for (size_t i = 0; i < projNode->columns.size(); ++i) {
            oss << projNode->columns[i];
            if (i + 1 < projNode->columns.size()) oss << ", ";
        }

        result.executionTrace.push_back("PROJECT [" + oss.str() + "]");
        return projTable;
    }

    // 3. SORT Operator
    if (const auto* sortNode = dynamic_cast<const SortNode*>(node)) {
        // Global sorting requires processing all candidate rows before slicing,
        // so early stopping is disabled (-1) for child operators.
        Table inputTable = executeNode(sortNode->child, symTable, result, -1);
        if (!result.success) return {};

        if (!inputTable.hasColumn(sortNode->columnName)) {
            result.success = false;
            result.errorMessage = "ORDER BY column '" + sortNode->columnName +
                                  "' does not exist in relation.";
            return {};
        }

        std::string sortCol = sortNode->columnName;
        bool asc = sortNode->ascending;

        std::stable_sort(inputTable.rows.begin(), inputTable.rows.end(),
                         [&sortCol, asc](const Row& a, const Row& b) {
                             Value va = a.getField(sortCol);
                             Value vb = b.getField(sortCol);
                             return asc ? (va < vb) : (vb < va);
                         });

        result.executionTrace.push_back("SORT " + sortCol + " " + (asc ? "ASC" : "DESC") +
                                        " → " + std::to_string(inputTable.rows.size()) + " rows sorted");
        return inputTable;
    }

    // 4. FILTER Operator
    if (const auto* filterNode = dynamic_cast<const FilterNode*>(node)) {
        if (filterNode->child && filterNode->child->getType() == IROpType::SCAN) {
            const auto* scanNode = dynamic_cast<const ScanNode*>(filterNode->child);
            size_t scannedRows = 0;
            return executeScanAndFilter(scanNode, filterNode->condition, symTable, earlyLimit, scannedRows, result);
        }

        Table inputTable = executeNode(filterNode->child, symTable, result, earlyLimit);
        if (!result.success) return {};

        Table filteredTable;
        filteredTable.name = inputTable.name;
        filteredTable.columnOrder = inputTable.columnOrder;

        for (const auto& row : inputTable.rows) {
            std::string err;
            Value match = evaluateExpr(filterNode->condition, row, err);
            if (!err.empty()) {
                result.success = false;
                result.errorMessage = err;
                return {};
            }
            if (match.boolVal) {
                filteredTable.rows.push_back(row);
            }
        }

        std::string condStr = filterNode->condition ? filterNode->condition->toString() : "";
        result.executionTrace.push_back("FILTER " + condStr + " → " +
                                        std::to_string(filteredTable.rows.size()) + " rows remain");
        return filteredTable;
    }

    // 5. SCAN Operator
    if (const auto* scanNode = dynamic_cast<const ScanNode*>(node)) {
        size_t scannedRows = 0;
        return executeScanAndFilter(scanNode, nullptr, symTable, earlyLimit, scannedRows, result);
    }

    result.success = false;
    result.errorMessage = "Unsupported IR node type during execution.";
    return {};
}

// ==========================================
// Main Execution Entrypoint
// ==========================================
ExecutionResult QueryExecutor::execute(const IRNode* plan, const SymbolTable& symTable) {
    ExecutionResult result;
    result.success = true;

    if (!plan) {
        result.success = false;
        result.errorMessage = "Cannot execute null IR plan.";
        return result;
    }

    // Safe Early Stopping determination:
    // Requires LIMIT clause and NO ORDER BY (SORT) clause.
    // If ORDER BY is present, global sorting over all matching candidate tuples must occur before slicing.
    int earlyLimit = -1;
    if (planHasOperator(plan, IROpType::LIMIT) && !planHasOperator(plan, IROpType::SORT)) {
        earlyLimit = planGetLimit(plan);
    }

    result.resultTable = executeNode(plan, symTable, result, earlyLimit);
    return result;
}
