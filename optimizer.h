#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <string>
#include <vector>
#include "ir.h"

/**
 * ============================================================================
 * EDUCATIONAL COMPILER PHASE: QUERY OPTIMIZATION
 * ============================================================================
 * Query optimization is the phase where the relational IR tree is inspected
 * and transformed into an algebraically equivalent but computationally more
 * efficient execution plan.
 * 
 * Rules implemented in SQLens:
 * 1. Filter Pushdown (Predicate Pushdown):
 *    Moves FILTER operations as close to the data source (SCAN) as possible,
 *    minimizing the number of rows processed by subsequent operators.
 * 
 * 2. Projection Optimization (Late Projection / Column Preservation):
 *    Ensures that PROJECT does not discard columns prematurely that are
 *    subsequently required by upstream operators such as SORT.
 * 
 * 3. Limit Optimization (Early Stopping vs Global Sort):
 *    Determines whether LIMIT can stop row generation early or if a global
 *    sort (ORDER BY) necessitates examining all candidate tuples first.
 * ============================================================================
 */

struct RuleReport {
    std::string ruleName;
    bool applied;
    std::string status;  // "APPLIED", "NOT REQUIRED", "NOT APPLICABLE"
    std::string details; // Human-readable explanation for compiler viva
};

struct OptimizationResult {
    const IRNode* originalPlan;
    IRNode* optimizedPlan;
    std::vector<RuleReport> reports;

    ~OptimizationResult();
    void print() const;
    std::string toJson() const;
};

class Optimizer {
public:
    // Analyzes and optimizes an IR plan, returning detailed transformation reports
    static OptimizationResult optimize(const IRNode* plan);

private:
    // Rule 1: Check and apply filter pushdown
    static bool applyFilterPushdown(IRNode*& root, RuleReport& report);

    // Rule 2: Check and apply projection reordering
    static bool applyProjectionReordering(IRNode*& root, RuleReport& report);

    // Rule 3: Check limit optimization applicability
    static void checkLimitOptimization(const IRNode* root, RuleReport& report);
};

#endif // OPTIMIZER_H
