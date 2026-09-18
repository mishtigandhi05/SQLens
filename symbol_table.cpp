#include "symbol_table.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>

std::string SymbolTable::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower;
}

SymbolTable::SymbolTable() {
    // Initial schema for Review 1: sample students table
    // id   -> INT
    // name -> STRING
    // cgpa -> FLOAT
    // age  -> INT
    schema["students"] = {
        {"id", "INT"},
        {"name", "STRING"},
        {"cgpa", "FLOAT"},
        {"age", "INT"}
    };

    // Maintain predictable order for deterministic educational display
    tableOrder = {"students"};
    columnOrder["students"] = {"id", "name", "cgpa", "age"};
}

bool SymbolTable::hasTable(const std::string& tableName) const {
    return schema.find(toLower(tableName)) != schema.end();
}

bool SymbolTable::hasColumn(const std::string& tableName, const std::string& colName) const {
    auto tIt = schema.find(toLower(tableName));
    if (tIt == schema.end()) {
        return false;
    }
    const auto& cols = tIt->second;
    return cols.find(toLower(colName)) != cols.end();
}

std::string SymbolTable::getColumnType(const std::string& tableName, const std::string& colName) const {
    auto tIt = schema.find(toLower(tableName));
    if (tIt == schema.end()) {
        return "";
    }
    const auto& cols = tIt->second;
    auto cIt = cols.find(toLower(colName));
    if (cIt == cols.end()) {
        return "";
    }
    return cIt->second;
}

void SymbolTable::printSymbolTable() const {
    for (size_t t = 0; t < tableOrder.size(); ++t) {
        const std::string& tableName = tableOrder[t];
        std::cout << "Table: " << tableName << "\n\n";
        std::cout << std::left << std::setw(14) << "Column" << "Data Type\n";
        std::cout << "-------------------------\n";

        auto orderIt = columnOrder.find(tableName);
        if (orderIt != columnOrder.end()) {
            for (const auto& col : orderIt->second) {
                std::string colType = getColumnType(tableName, col);
                std::cout << std::left << std::setw(14) << col << colType << "\n";
            }
        }
        if (t + 1 < tableOrder.size()) {
            std::cout << "\n";
        }
    }
}
