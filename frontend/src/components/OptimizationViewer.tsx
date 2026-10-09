import React from 'react';
import { OptimizerJson, IrNodeJson } from '../types/compiler';
import { ArrowDown, Check, Minus } from 'lucide-react';

interface OptimizationViewerProps {
  optimization: OptimizerJson | null | undefined;
}

const linearizePlan = (node: IrNodeJson | null | undefined): IrNodeJson[] => {
  const result: IrNodeJson[] = [];
  let curr = node;
  while (curr) {
    result.push(curr);
    curr = curr.child;
  }
  return result;
};

export const OptimizationViewer: React.FC<OptimizationViewerProps> = ({ optimization }) => {
  if (!optimization) {
    return (
      <div className="empty-stage-state">
        <p>No optimization data available. Execute a query to inspect optimization rules.</p>
      </div>
    );
  }

  const { originalPlan, optimizedPlan, rules } = optimization;
  const origNodes = linearizePlan(originalPlan);
  const optNodes = linearizePlan(optimizedPlan);

  const getStatusBadge = (status: string) => {
    switch (status) {
      case 'APPLIED':
        return {
          icon: <Check size={12} strokeWidth={2.5} />,
          className: 'rule-status-applied',
          label: 'Applied',
        };
      case 'NOT REQUIRED':
        return {
          icon: <Minus size={12} strokeWidth={2.5} />,
          className: 'rule-status-neutral',
          label: 'Not Required',
        };
      case 'NOT APPLICABLE':
      default:
        return {
          icon: <Minus size={12} strokeWidth={2.5} />,
          className: 'rule-status-neutral',
          label: 'Not Applicable',
        };
    }
  };

  return (
    <div className="optimization-view">
      {/* Side-by-Side Plan Diff */}
      <div className="plans-diff-container">
        {/* Original Plan Column */}
        <div className="plan-column">
          <div className="plan-column-header">
            <span className="plan-tag">Original</span>
            <span className="plan-name">Canonical Relational Plan</span>
          </div>

          <div className="plan-nodes-stack">
            {origNodes.map((node, idx) => (
              <React.Fragment key={idx}>
                <div className="plan-diff-node">
                  <span className="diff-node-op">{node.op}</span>
                  {node.details && (
                    <span className="diff-node-details font-mono">[{node.details}]</span>
                  )}
                </div>
                {idx < origNodes.length - 1 && (
                  <div className="diff-arrow">
                    <ArrowDown size={12} />
                  </div>
                )}
              </React.Fragment>
            ))}
          </div>
        </div>

        {/* Optimized Plan Column */}
        <div className="plan-column">
          <div className="plan-column-header">
            <span className="plan-tag tag-optimized">Optimized</span>
            <span className="plan-name">Rewritten Relational Plan</span>
          </div>

          <div className="plan-nodes-stack">
            {optNodes.map((node, idx) => (
              <React.Fragment key={idx}>
                <div className="plan-diff-node node-optimized">
                  <span className="diff-node-op">{node.op}</span>
                  {node.details && (
                    <span className="diff-node-details font-mono">[{node.details}]</span>
                  )}
                </div>
                {idx < optNodes.length - 1 && (
                  <div className="diff-arrow">
                    <ArrowDown size={12} />
                  </div>
                )}
              </React.Fragment>
            ))}
          </div>
        </div>
      </div>

      {/* Rules Breakdown */}
      <div className="rules-section">
        <div className="rules-header">
          <span>Optimization Rules Evaluated</span>
        </div>

        <div className="rules-list">
          {rules?.map((rule, idx) => {
            const badge = getStatusBadge(rule.status);
            return (
              <div key={idx} className="rule-card">
                <div className="rule-top">
                  <span className="rule-name">{rule.ruleName}</span>
                  <span className={`rule-badge ${badge.className}`}>
                    {badge.icon}
                    <span>{badge.label}</span>
                  </span>
                </div>
                <p className="rule-explanation">{rule.details}</p>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
};
