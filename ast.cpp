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

// ==========================================
// ColumnRefNode
// ==========================================
ColumnRefNode::ColumnRefNode(const std::string& name) : name(name) {}

void ColumnRefNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << name << "\n";
}

// ==========================================
// LiteralNode
// ==========================================
LiteralNode::LiteralNode(const std::string& value, const std::string& type)
    : value(value), type(type) {}

void LiteralNode::print(const std::string& prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "`-- " : "|-- ") << value << "\n";
}

// ==========================================
// SelectQueryNode
// ==========================================
SelectQueryNode::SelectQueryNode(const std::vector<std::string>& cols,
                                 const std::string& table,
                                 ExprNode* whereExpr,
                                 const std::string& orderBy,
                                 int limit)
    : columns(cols), tableName(table), whereClause(whereExpr),
      orderByColumn(orderBy), limitValue(limit) {}

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
        std::cout << orderPrefix << "`-- " << orderByColumn << "\n";
    }

    // Section 5: LIMIT (optional)
    if (hasLimit) {
        std::cout << "`-- LIMIT\n";
        std::cout << "    `-- " << limitValue << "\n";
    }
}
