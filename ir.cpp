#include "ir.h"
#include <sstream>
#include <algorithm>

// ==========================================
// Helper: Extract column references
// ==========================================
void collectColumnsFromExpr(const ExprNode* expr, std::vector<std::string>& outCols) {
    if (!expr) return;
    if (const auto* colNode = dynamic_cast<const ColumnRefNode*>(expr)) {
        if (std::find(outCols.begin(), outCols.end(), colNode->name) == outCols.end()) {
            outCols.push_back(colNode->name);
        }
    } else if (const auto* binNode = dynamic_cast<const BinaryOpNode*>(expr)) {
        collectColumnsFromExpr(binNode->left, outCols);
        collectColumnsFromExpr(binNode->right, outCols);
    }
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

// ==========================================
// ScanNode
// ==========================================
ScanNode::ScanNode(const std::string& table) : tableName(table) {}

std::string ScanNode::toString() const {
    return "SCAN [" + tableName + "]";
}

void ScanNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << toString() << "\n";
}

std::string ScanNode::toJson() const {
    return "{\"op\":\"SCAN\",\"tableName\":\"" + escapeJson(tableName) + "\",\"details\":\"" + escapeJson(tableName) + "\",\"child\":null}";
}

IRNode* ScanNode::clone() const {
    return new ScanNode(tableName);
}

// ==========================================
// FilterNode
// ==========================================
FilterNode::FilterNode(IRNode* child, ExprNode* cond)
    : child(child), condition(cond) {}

FilterNode::~FilterNode() {
    delete child;
    delete condition;
}

std::string FilterNode::toString() const {
    return "FILTER [" + (condition ? condition->toString() : "") + "]";
}

void FilterNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << toString() << "\n";
    if (child) {
        child->print(prefix + (isLast ? "    " : "|   "), true);
    }
}

std::string FilterNode::toJson() const {
    std::string condStr = condition ? condition->toString() : "";
    std::string json = "{\"op\":\"FILTER\",\"details\":\"" + escapeJson(condStr) + "\"";
    if (child) {
        json += ",\"child\":" + child->toJson();
    } else {
        json += ",\"child\":null";
    }
    json += "}";
    return json;
}

IRNode* FilterNode::clone() const {
    return new FilterNode(child ? child->clone() : nullptr, condition ? condition->clone() : nullptr);
}

// ==========================================
// ProjectNode
// ==========================================
ProjectNode::ProjectNode(IRNode* child, const std::vector<std::string>& cols)
    : child(child), columns(cols) {}

ProjectNode::~ProjectNode() {
    delete child;
}

std::string ProjectNode::toString() const {
    std::ostringstream oss;
    oss << "PROJECT [";
    for (size_t i = 0; i < columns.size(); ++i) {
        oss << columns[i];
        if (i + 1 < columns.size()) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

void ProjectNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << toString() << "\n";
    if (child) {
        child->print(prefix + (isLast ? "    " : "|   "), true);
    }
}

std::string ProjectNode::toJson() const {
    std::string colsStr;
    for (size_t i = 0; i < columns.size(); ++i) {
        colsStr += columns[i];
        if (i + 1 < columns.size()) colsStr += ", ";
    }
    std::string json = "{\"op\":\"PROJECT\",\"details\":\"" + escapeJson(colsStr) + "\",\"columns\":[";
    for (size_t i = 0; i < columns.size(); ++i) {
        json += "\"" + escapeJson(columns[i]) + "\"";
        if (i + 1 < columns.size()) json += ",";
    }
    json += "]";
    if (child) {
        json += ",\"child\":" + child->toJson();
    } else {
        json += ",\"child\":null";
    }
    json += "}";
    return json;
}

IRNode* ProjectNode::clone() const {
    return new ProjectNode(child ? child->clone() : nullptr, columns);
}

// ==========================================
// SortNode
// ==========================================
SortNode::SortNode(IRNode* child, const std::string& col, bool asc)
    : child(child), columnName(col), ascending(asc) {}

SortNode::~SortNode() {
    delete child;
}

std::string SortNode::toString() const {
    return "SORT [" + columnName + (ascending ? " ASC" : " DESC") + "]";
}

void SortNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << toString() << "\n";
    if (child) {
        child->print(prefix + (isLast ? "    " : "|   "), true);
    }
}

std::string SortNode::toJson() const {
    std::string details = columnName + (ascending ? " ASC" : " DESC");
    std::string json = "{\"op\":\"SORT\",\"column\":\"" + escapeJson(columnName) + "\",\"ascending\":" + std::string(ascending ? "true" : "false") + ",\"details\":\"" + escapeJson(details) + "\"";
    if (child) {
        json += ",\"child\":" + child->toJson();
    } else {
        json += ",\"child\":null";
    }
    json += "}";
    return json;
}

IRNode* SortNode::clone() const {
    return new SortNode(child ? child->clone() : nullptr, columnName, ascending);
}

// ==========================================
// LimitNode
// ==========================================
LimitNode::LimitNode(IRNode* child, int limit)
    : child(child), limitValue(limit) {}

LimitNode::~LimitNode() {
    delete child;
}

std::string LimitNode::toString() const {
    return "LIMIT [" + std::to_string(limitValue) + "]";
}

void LimitNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << toString() << "\n";
    if (child) {
        child->print(prefix + (isLast ? "    " : "|   "), true);
    }
}

std::string LimitNode::toJson() const {
    std::string json = "{\"op\":\"LIMIT\",\"limit\":" + std::to_string(limitValue) + ",\"details\":\"" + std::to_string(limitValue) + "\"";
    if (child) {
        json += ",\"child\":" + child->toJson();
    } else {
        json += ",\"child\":null";
    }
    json += "}";
    return json;
}

IRNode* LimitNode::clone() const {
    return new LimitNode(child ? child->clone() : nullptr, limitValue);
}

// ==========================================
// IRBuilder
// ==========================================
IRNode* IRBuilder::buildFromAST(const SelectQueryNode* ast) {
    if (!ast) return nullptr;

    // Bottom-up construction of canonical relational pipeline:
    // 1. SCAN base table
    IRNode* plan = new ScanNode(ast->tableName);

    // 2. FILTER selection (if WHERE clause is present)
    if (ast->whereClause) {
        plan = new FilterNode(plan, ast->whereClause->clone());
    }

    // 3. PROJECT column list
    plan = new ProjectNode(plan, ast->columns);

    // 4. SORT ordering (if ORDER BY is present)
    if (!ast->orderByColumn.empty()) {
        plan = new SortNode(plan, ast->orderByColumn, ast->orderByAscending);
    }

    // 5. LIMIT row count (if LIMIT is present)
    if (ast->limitValue >= 0) {
        plan = new LimitNode(plan, ast->limitValue);
    }

    return plan;
}
