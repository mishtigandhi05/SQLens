import React, { useState } from 'react';
import { IrNodeJson } from '../types/compiler';
import { ArrowDown } from 'lucide-react';

interface IrViewerProps {
  plan: IrNodeJson | null | undefined;
}

const OP_DESCRIPTIONS: Record<string, string> = {
  SCAN: 'Reads records from data/students.csv relation',
  FILTER: 'Evaluates row selection predicate',
  PROJECT: 'Slices attributes and outputs target columns',
  SORT: 'Sorts tuples by specified ordering attribute',
  LIMIT: 'Restricts total emitted tuples count',
};

export const IrViewer: React.FC<IrViewerProps> = ({ plan }) => {
  const [viewMode, setViewMode] = useState<'flow' | 'json'>('flow');

  if (!plan) {
    return (
      <div className="empty-stage-state">
        <p>No Intermediate Representation available. Execute a valid query to generate the relational plan.</p>
      </div>
    );
  }

  // Linearize the unary relational chain (LIMIT -> SORT -> PROJECT -> FILTER -> SCAN)
  const linearNodes: IrNodeJson[] = [];
  let curr: IrNodeJson | null | undefined = plan;
  while (curr) {
    linearNodes.push(curr);
    curr = curr.child;
  }

  return (
    <div className="ir-view">
      <div className="ir-toolbar">
        <span className="ir-toolbar-title">Relational Algebra Operator Tree</span>
        <div className="view-mode-toggle">
          <button
            type="button"
            className={`mode-btn ${viewMode === 'flow' ? 'active' : ''}`}
            onClick={() => setViewMode('flow')}
          >
            Plan Flow
          </button>
          <button
            type="button"
            className={`mode-btn ${viewMode === 'json' ? 'active' : ''}`}
            onClick={() => setViewMode('json')}
          >
            Raw JSON
          </button>
        </div>
      </div>

      {viewMode === 'json' ? (
        <div className="code-block-container">
          <pre className="code-pre font-mono">
            {JSON.stringify(plan, null, 2)}
          </pre>
        </div>
      ) : (
        <div className="ir-flow-canvas">
          <div className="ir-nodes-column">
            {linearNodes.map((node, idx) => {
              const isLast = idx === linearNodes.length - 1;
              const desc = OP_DESCRIPTIONS[node.op] || 'Relational operation';

              return (
                <React.Fragment key={idx}>
                  <div className="ir-plan-node">
                    <div className="node-badge-col">
                      <span className="op-tag">{node.op}</span>
                    </div>

                    <div className="node-detail-col">
                      <div className="node-details font-mono">
                        {node.details ? `[ ${node.details} ]` : ''}
                      </div>
                      <div className="node-desc text-muted">{desc}</div>
                    </div>
                  </div>

                  {!isLast && (
                    <div className="ir-connector-line">
                      <ArrowDown size={14} className="connector-arrow" />
                    </div>
                  )}
                </React.Fragment>
              );
            })}
          </div>
        </div>
      )}
    </div>
  );
};
