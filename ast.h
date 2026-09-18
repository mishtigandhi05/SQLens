#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

// Portable string duplication function
inline char* my_strdup(const char* s) {
    if (!s) return nullptr;
    size_t len = std::strlen(s);
    char* copy = (char*)std::malloc(len + 1);
    if (copy) {
        std::memcpy(copy, s, len + 1);
    }
    return copy;
}

// Forward declaration
class ExprNode;

/**
 * @brief Base class for all Abstract Syntax Tree nodes.
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    // Prints the node and its subtrees with ASCII tree branches
    virtual void print(const std::string& prefix = "", bool isLast = true) const = 0;
};

/**
 * @brief Base class for expressions (used in WHERE conditions, columns, literals).
 */
class ExprNode : public ASTNode {
public:
    virtual ~ExprNode() = default;
};

/**
 * @brief Represents a binary operation (e.g. cgpa > 8.0, age >= 18, cond1 AND cond2).
 */
class BinaryOpNode : public ExprNode {
public:
    std::string op;
    ExprNode* left;
    ExprNode* right;

    BinaryOpNode(const std::string& op, ExprNode* left, ExprNode* right);
    ~BinaryOpNode() override;

    void print(const std::string& prefix = "", bool isLast = true) const override;
};

/**
 * @brief Represents a column reference identifier (e.g. name, cgpa, age).
 */
class ColumnRefNode : public ExprNode {
public:
    std::string name;

    explicit ColumnRefNode(const std::string& name);
    void print(const std::string& prefix = "", bool isLast = true) const override;
};

/**
 * @brief Represents a literal constant (e.g. 8.0, 20, 'Alice', TRUE).
 */
class LiteralNode : public ExprNode {
public:
    std::string value;
    std::string type; // "INT", "FLOAT", "STRING", "BOOLEAN"

    LiteralNode(const std::string& value, const std::string& type);
    void print(const std::string& prefix = "", bool isLast = true) const override;
};

/**
 * @brief Represents the complete parsed SELECT query.
 */
class SelectQueryNode : public ASTNode {
public:
    std::vector<std::string> columns;
    std::string tableName;
    ExprNode* whereClause;
    std::string orderByColumn;
    int limitValue; // -1 if LIMIT is not present

    SelectQueryNode(const std::vector<std::string>& cols,
                    const std::string& table,
                    ExprNode* whereExpr = nullptr,
                    const std::string& orderBy = "",
                    int limit = -1);
    ~SelectQueryNode() override;

    void print(const std::string& prefix = "", bool isLast = true) const override;
};

// Global root pointer populated by Bison parser
extern SelectQueryNode* g_root_ast;

#endif // AST_H
