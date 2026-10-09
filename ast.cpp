#include "ast.h"
#include <iostream>

// Global root pointer
SelectQueryNode* g_root_ast = nullptr;

// ==========================================
// BinaryOpNode
// ==========================================
BinaryOpNode::BinaryOpNode(const std::string& op, ExprNode* left, ExprNode* right)
    : op(op), left(left), right(right) {}

BinaryOpNode::~BinaryOpNode() {
    delete left;
    delete right;
}

void BinaryOpNode::print(const std::string& prefix, bool isLast) const {
    // Branch connector using ASCII characters
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << "CONDITION\n";

    std::string childPrefix = prefix + (isLast ? "    " : "|   ");

    // 1st child: Operator
    std::cout << childPrefix << "|-- " << op << "\n";

    // 2nd child: Left operand
    if (left) {
        left->print(childPrefix, false);
    }

    // 3rd child: Right operand (last child)
    if (right) {
        right->print(childPrefix, true);
    }
}

ExprNode* BinaryOpNode::clone() const {
    return new BinaryOpNode(op, left ? left->clone() : nullptr, right ? right->clone() : nullptr);
}

std::string BinaryOpNode::toString() const {
    std::string lStr = left ? left->toString() : "";
    std::string rStr = right ? right->toString() : "";
    return lStr + " " + op + " " + rStr;
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

std::string BinaryOpNode::toJson() const {
    std::string json = "{\"type\":\"BinaryOp\",\"op\":\"" + escapeJson(op) + "\"";
    if (left) {
        json += ",\"left\":" + left->toJson();
    } else {
        json += ",\"left\":null";
    }
    if (right) {
        json += ",\"right\":" + right->toJson();
    } else {
        json += ",\"right\":null";
    }
    json += "}";
    return json;
}

// ==========================================
// ColumnRefNode
// ==========================================
ColumnRefNode::ColumnRefNode(const std::string& name) : name(name) {}

void ColumnRefNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << name << "\n";
}

ExprNode* ColumnRefNode::clone() const {
    return new ColumnRefNode(name);
}

std::string ColumnRefNode::toString() const {
    return name;
}

std::string ColumnRefNode::toJson() const {
    return "{\"type\":\"ColumnRef\",\"name\":\"" + escapeJson(name) + "\"}";
}

// ==========================================
// LiteralNode
// ==========================================
LiteralNode::LiteralNode(const std::string& value, const std::string& type)
    : value(value), type(type) {}

void LiteralNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << value << "\n";
}

ExprNode* LiteralNode::clone() const {
    return new LiteralNode(value, type);
}

std::string LiteralNode::toString() const {
    if (type == "STRING") {
        return "'" + value + "'";
    }
    return value;
}

std::string LiteralNode::toJson() const {
    return "{\"type\":\"Literal\",\"value\":\"" + escapeJson(value) + "\",\"literalType\":\"" + escapeJson(type) + "\"}";
}

// ==========================================
// SelectQueryNode
// ==========================================
SelectQueryNode::SelectQueryNode(const std::vector<std::string>& cols,
                                 const std::string& table,
                                 ExprNode* whereExpr,
                                 const std::string& orderBy,
                                 bool orderAsc,
                                 int limit)
    : columns(cols), tableName(table), whereClause(whereExpr),
      orderByColumn(orderBy), orderByAscending(orderAsc), limitValue(limit) {}

SelectQueryNode::~SelectQueryNode() {
    delete whereClause;
}

void SelectQueryNode::print(const std::string& prefix, bool isLast) const {
    (void)prefix;
    (void)isLast;

    std::cout << "SELECT\n";

    // Determine how many top-level sections exist to set `isLast` properly
    bool hasWhere = (whereClause != nullptr);
    bool hasOrderBy = (!orderByColumn.empty());
    bool hasLimit = (limitValue >= 0);

    // Section 1: COLUMNS (always present in valid query)
    bool columnsIsLast = (!hasWhere && !hasOrderBy && !hasLimit && tableName.empty());
    std::cout << (columnsIsLast ? "`-- " : "|-- ") << "COLUMNS\n";
    std::string colPrefix = (columnsIsLast ? "    " : "|   ");
    for (size_t i = 0; i < columns.size(); ++i) {
        bool lastCol = (i == columns.size() - 1);
        std::cout << colPrefix << (lastCol ? "`-- " : "|-- ") << columns[i] << "\n";
    }

    // Section 2: FROM (always present)
    bool fromIsLast = (!hasWhere && !hasOrderBy && !hasLimit);
    std::cout << (fromIsLast ? "`-- " : "|-- ") << "FROM\n";
    std::string fromPrefix = (fromIsLast ? "    " : "|   ");
    std::cout << fromPrefix << "`-- " << tableName << "\n";

    // Section 3: WHERE (optional)
    if (hasWhere) {
        bool whereIsLast = (!hasOrderBy && !hasLimit);
        std::cout << (whereIsLast ? "`-- " : "|-- ") << "WHERE\n";
        std::string wherePrefix = (whereIsLast ? "    " : "|   ");
        whereClause->print(wherePrefix, true);
    }

    // Section 4: ORDER BY (optional)
    if (hasOrderBy) {
        bool orderIsLast = (!hasLimit);
        std::cout << (orderIsLast ? "`-- " : "|-- ") << "ORDER BY\n";
        std::string orderPrefix = (orderIsLast ? "    " : "|   ");
        std::cout << orderPrefix << "`-- " << orderByColumn << (orderByAscending ? " ASC" : " DESC") << "\n";
    }

    // Section 5: LIMIT (optional)
    if (hasLimit) {
        std::cout << "`-- LIMIT\n";
        std::cout << "    `-- " << limitValue << "\n";
    }
}

std::string SelectQueryNode::toJson() const {
    std::string json = "{\"type\":\"SelectQuery\",\"table\":\"" + escapeJson(tableName) + "\",\"columns\":[";
    for (size_t i = 0; i < columns.size(); ++i) {
        json += "\"" + escapeJson(columns[i]) + "\"";
        if (i + 1 < columns.size()) json += ",";
    }
    json += "]";
    if (whereClause) {
        json += ",\"where\":" + whereClause->toJson();
    } else {
        json += ",\"where\":null";
    }
    if (!orderByColumn.empty()) {
        json += ",\"orderBy\":\"" + escapeJson(orderByColumn) + "\"";
        json += ",\"orderAsc\":" + std::string(orderByAscending ? "true" : "false");
    } else {
        json += ",\"orderBy\":null,\"orderAsc\":true";
    }
    json += ",\"limit\":" + std::to_string(limitValue);
    json += "}";
    return json;
}
