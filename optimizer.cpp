#include "optimizer.h"
#include <iostream>
#include <algorithm>

OptimizationResult::~OptimizationResult() {
    if (optimizedPlan && optimizedPlan != originalPlan) {
        delete optimizedPlan;
        optimizedPlan = nullptr;
    }
}

void OptimizationResult::print() const {
    std::cout << "Original Plan:\n";
    if (originalPlan) {
        originalPlan->print();
    } else {
        std::cout << "(Empty Plan)\n";
    }

    std::cout << "\nOptimization Rules:\n";
    std::cout << "- Filter Pushdown (Predicate Pushdown)\n";
    std::cout << "- Projection Reordering (Late Projection / Column Preservation)\n";
    std::cout << "- Limit Optimization (Early Stopping vs. Global Sort)\n\n";

    std::cout << "Applied Transformations:\n";
    for (const auto& r : reports) {
        std::cout << "- " << r.ruleName << ": " << r.status;
        if (!r.details.empty()) {
            std::cout << " (" << r.details << ")";
        }
        std::cout << "\n";
    }

    std::cout << "\nOptimized Plan:\n";
    if (optimizedPlan) {
        optimizedPlan->print();
    } else {
        std::cout << "(Empty Plan)\n";
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

std::string OptimizationResult::toJson() const {
    std::string json = "{";
    json += "\"originalPlan\":" + (originalPlan ? originalPlan->toJson() : "null");
    json += ",\"optimizedPlan\":" + (optimizedPlan ? optimizedPlan->toJson() : "null");
    json += ",\"rules\":[";
    for (size_t i = 0; i < reports.size(); ++i) {
        json += "{\"ruleName\":\"" + escapeJson(reports[i].ruleName) + "\"";
        json += ",\"applied\":" + std::string(reports[i].applied ? "true" : "false");
        json += ",\"status\":\"" + escapeJson(reports[i].status) + "\"";
        json += ",\"details\":\"" + escapeJson(reports[i].details) + "\"}";
        if (i + 1 < reports.size()) json += ",";
    }
    json += "]}";
    return json;
}

// ============================================================================
// Helper: Case-insensitive string utility
// ============================================================================
static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower;
}

// ============================================================================
// Helper: Check if an operator subtree contains a specific OpType
// ============================================================================
static bool hasOperator(const IRNode* node, IROpType type) {
    if (!node) return false;
    if (node->getType() == type) return true;
    return hasOperator(node->getChild(), type);
}

// ============================================================================
// Rule 1: Filter Pushdown (Predicate Pushdown)
// ============================================================================
bool Optimizer::applyFilterPushdown(IRNode*& root, RuleReport& report) {
    report.ruleName = "Filter Pushdown";

    if (!hasOperator(root, IROpType::FILTER)) {
        report.applied = false;
        report.status = "NOT APPLICABLE";
        report.details = "No WHERE clause in query";
        return false;
    }

    // Traversal looking for FILTER and its parent
    IRNode* filterParent = nullptr;
    IRNode* curr = root;
    FilterNode* filterNode = nullptr;

    while (curr) {
        if (curr->getType() == IROpType::FILTER) {
            filterNode = dynamic_cast<FilterNode*>(curr);
            break;
        }
        filterParent = curr;
        curr = curr->getChild();
    }

    if (!filterNode) {
        report.applied = false;
        report.status = "NOT APPLICABLE";
        report.details = "No WHERE clause in query";
        return false;
    }

    // Check if FILTER is already immediately above SCAN
    if (filterNode->child && filterNode->child->getType() == IROpType::SCAN) {
        // Filter is already directly on top of SCAN (optimal for single-table query)
        report.applied = false;
        report.status = "NOT REQUIRED";
        report.details = "Filter is already adjacent to SCAN";
        return false;
    }

    // If FILTER is NOT adjacent to SCAN, actually transform the IR tree by pushing FILTER down to SCAN
    // 1. Detach filterNode from its current position
    if (filterParent == nullptr) {
        root = filterNode->child;
    } else {
        filterParent->setChild(filterNode->child);
    }

    // 2. Find ScanNode and its parent in the remaining tree
    IRNode* scanParent = nullptr;
    IRNode* scanSearch = root;
    while (scanSearch && scanSearch->getType() != IROpType::SCAN) {
        scanParent = scanSearch;
        scanSearch = scanSearch->getChild();
    }

    if (scanSearch && scanParent) {
        // Splice filterNode immediately above scanSearch (the ScanNode)
        filterNode->child = scanSearch;
        scanParent->setChild(filterNode);

        report.applied = true;
        report.status = "APPLIED";
        report.details = "Pushed FILTER adjacent to SCAN for early tuple reduction";
        return true;
    }

    report.applied = false;
    report.status = "NOT REQUIRED";
    report.details = "Filter is already adjacent to SCAN";
    return false;
}

// ============================================================================
// Rule 2: Projection Reordering (Late Projection / Column Preservation)
// ============================================================================
bool Optimizer::applyProjectionReordering(IRNode*& root, RuleReport& report) {
    report.ruleName = "Projection Reordering";

    if (!root) {
        report.applied = false;
        report.status = "NOT REQUIRED";
        report.details = "Empty plan";
        return false;
    }

    // Find if SORT appears directly above PROJECT in the tree
    IRNode* parentOfSort = nullptr;
    IRNode* curr = root;
    SortNode* sortNode = nullptr;

    while (curr) {
        if (curr->getType() == IROpType::SORT) {
            sortNode = dynamic_cast<SortNode*>(curr);
            break;
        }
        parentOfSort = curr;
        curr = curr->getChild();
    }

    if (!sortNode || !sortNode->child || sortNode->child->getType() != IROpType::PROJECT) {
        report.applied = false;
        report.status = "NOT REQUIRED";
        report.details = "Projection placement is safe and optimal";
        return false;
    }

    ProjectNode* projNode = dynamic_cast<ProjectNode*>(sortNode->child);
    if (!projNode) {
        report.applied = false;
        report.status = "NOT REQUIRED";
        report.details = "Projection placement is safe and optimal";
        return false;
    }

    // Check if the ORDER BY column is already included in the SELECT projection list
    bool sortColInProject = false;
    std::string targetCol = toLower(sortNode->columnName);
    for (const auto& col : projNode->columns) {
        if (toLower(col) == targetCol) {
            sortColInProject = true;
            break;
        }
    }

    if (sortColInProject) {
        // Sort key is already present in projected columns; PROJECT does not drop it
        report.applied = false;
        report.status = "NOT REQUIRED";
        report.details = "Sort key '" + sortNode->columnName + "' is already present in projection list";
        return false;
    }

    // The sort column is NOT in the SELECT projection list!
    // Defer PROJECT above SORT so that SORT has full access to the sort column.
    IRNode* subChild = projNode->child;
    sortNode->child = subChild;
    projNode->child = sortNode;

    if (parentOfSort == nullptr) {
        root = projNode;
    } else {
        parentOfSort->setChild(projNode);
    }

    report.applied = true;
    report.status = "APPLIED";
    report.details = "Deferred PROJECT above SORT to preserve sort key '" + sortNode->columnName + "'";
    return true;
}

// ============================================================================
// Rule 3: Limit Optimization (Early Stopping vs. Global Sort)
// ============================================================================
void Optimizer::checkLimitOptimization(const IRNode* root, RuleReport& report) {
    report.ruleName = "Limit Optimization";

    if (!hasOperator(root, IROpType::LIMIT)) {
        report.applied = false;
        report.status = "NOT REQUIRED";
        report.details = "No LIMIT clause in query";
        return;
    }

    if (hasOperator(root, IROpType::SORT)) {
        report.applied = false;
        report.status = "NOT APPLICABLE";
        report.details = "ORDER BY requires global sorting over all matching tuples before LIMIT";
    } else {
        report.applied = true;
        report.status = "APPLIED";
        report.details = "Early stopping enabled: scan can terminate once row limit is satisfied";
    }
}

// ============================================================================
// Main Optimization Entrypoint
// ============================================================================
OptimizationResult Optimizer::optimize(const IRNode* plan) {
    OptimizationResult result;
    result.originalPlan = plan;
    result.optimizedPlan = plan ? plan->clone() : nullptr;

    if (!result.optimizedPlan) {
        return result;
    }

    RuleReport r1, r2, r3;
    applyFilterPushdown(result.optimizedPlan, r1);
    applyProjectionReordering(result.optimizedPlan, r2);
    checkLimitOptimization(result.optimizedPlan, r3);

    result.reports.push_back(r1);
    result.reports.push_back(r2);
    result.reports.push_back(r3);

    return result;
}
