#ifndef IR_H
#define IR_H

#include <iostream>
#include <string>
#include <vector>
#include "ast.h"

/**
 * ============================================================================
 * EDUCATIONAL COMPILER PHASE: INTERMEDIATE REPRESENTATION (IR)
 * ============================================================================
 * In compiler design, Intermediate Representation bridges frontend parsing/analysis
 * with backend optimization and code generation/execution.
 * 
 * In SQLens (an educational relational compiler), the IR is a Relational Algebra
 * Operator Tree representing dataflow operations:
 * - SCAN:    Table scan retrieving raw records from disk/CSV.
 * - FILTER:  Selection (sigma) applying row-level predicate conditions.
 * - PROJECT: Projection (pi) selecting desired output attributes.
 * - SORT:    Ordering (tau) sorting tuples by specified attribute.
 * - LIMIT:   Slicing limiting the maximum number of emitted tuples.
 * ============================================================================
 */

enum class IROpType {
    SCAN,
    FILTER,
    PROJECT,
    SORT,
    LIMIT
};

/**
 * @brief Base class for all relational algebra IR nodes.
 */
class IRNode {
public:
    virtual ~IRNode() = default;

    virtual IROpType getType() const = 0;
    virtual std::string getOpName() const = 0;
    virtual std::string toString() const = 0;

    // Prints the IR operator tree with ASCII branches
    virtual void print(const std::string& prefix = "", bool isLast = true) const = 0;

    // Serializes relational operator tree to JSON for web visualizer
    virtual std::string toJson() const = 0;

    // Deep copy clone
    virtual IRNode* clone() const = 0;

    // Child access for unary relational operators
    virtual IRNode* getChild() const { return nullptr; }
    virtual void setChild(IRNode* newChild) { (void)newChild; }
};

/**
 * @brief SCAN operator: Reads records from a named table.
 */
class ScanNode : public IRNode {
public:
    std::string tableName;

    explicit ScanNode(const std::string& table);
    ~ScanNode() override = default;

    IROpType getType() const override { return IROpType::SCAN; }
    std::string getOpName() const override { return "SCAN"; }
    std::string toString() const override;
    void print(const std::string& prefix = "", bool isLast = true) const override;
    std::string toJson() const override;
    IRNode* clone() const override;
};

/**
 * @brief FILTER operator: Selects rows that satisfy the given condition expression.
 */
class FilterNode : public IRNode {
public:
    IRNode* child;
    ExprNode* condition; // Owned by FilterNode

    FilterNode(IRNode* child, ExprNode* cond);
    ~FilterNode() override;

    IROpType getType() const override { return IROpType::FILTER; }
    std::string getOpName() const override { return "FILTER"; }
    std::string toString() const override;
    void print(const std::string& prefix = "", bool isLast = true) const override;
    std::string toJson() const override;
    IRNode* clone() const override;

    IRNode* getChild() const override { return child; }
    void setChild(IRNode* newChild) override { child = newChild; }
};

/**
 * @brief PROJECT operator: Selects and orders specific columns from input rows.
 */
class ProjectNode : public IRNode {
public:
    IRNode* child;
    std::vector<std::string> columns;

    ProjectNode(IRNode* child, const std::vector<std::string>& cols);
    ~ProjectNode() override;

    IROpType getType() const override { return IROpType::PROJECT; }
    std::string getOpName() const override { return "PROJECT"; }
    std::string toString() const override;
    void print(const std::string& prefix = "", bool isLast = true) const override;
    std::string toJson() const override;
    IRNode* clone() const override;

    IRNode* getChild() const override { return child; }
    void setChild(IRNode* newChild) override { child = newChild; }
};

/**
 * @brief SORT operator: Orders incoming tuples by a specified attribute.
 */
class SortNode : public IRNode {
public:
    IRNode* child;
    std::string columnName;
    bool ascending;

    SortNode(IRNode* child, const std::string& col, bool asc = true);
    ~SortNode() override;

    IROpType getType() const override { return IROpType::SORT; }
    std::string getOpName() const override { return "SORT"; }
    std::string toString() const override;
    void print(const std::string& prefix = "", bool isLast = true) const override;
    std::string toJson() const override;
    IRNode* clone() const override;

    IRNode* getChild() const override { return child; }
    void setChild(IRNode* newChild) override { child = newChild; }
};

/**
 * @brief LIMIT operator: Restricts the total number of output tuples.
 */
class LimitNode : public IRNode {
public:
    IRNode* child;
    int limitValue;

    LimitNode(IRNode* child, int limit);
    ~LimitNode() override;

    IROpType getType() const override { return IROpType::LIMIT; }
    std::string getOpName() const override { return "LIMIT"; }
    std::string toString() const override;
    void print(const std::string& prefix = "", bool isLast = true) const override;
    std::string toJson() const override;
    IRNode* clone() const override;

    IRNode* getChild() const override { return child; }
    void setChild(IRNode* newChild) override { child = newChild; }
};

/**
 * @brief IRBuilder transforms an AST SelectQueryNode into an initial Relational IR plan.
 */
class IRBuilder {
public:
    static IRNode* buildFromAST(const SelectQueryNode* ast);
};

// Helper function to extract all column names referenced in an expression
void collectColumnsFromExpr(const ExprNode* expr, std::vector<std::string>& outCols);

#endif // IR_H
