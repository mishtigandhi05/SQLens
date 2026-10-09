import React, { useState } from 'react';
import { AstNodeJson } from '../types/compiler';
import { ChevronRight, ChevronDown } from 'lucide-react';

interface AstViewerProps {
  ast: AstNodeJson | null;
}

interface TreeNodeProps {
  label: string;
  badge?: string;
  children?: React.ReactNode;
  defaultExpanded?: boolean;
}

const TreeNode: React.FC<TreeNodeProps> = ({
  label,
  badge,
  children,
  defaultExpanded = true,
}) => {
  const [expanded, setExpanded] = useState(defaultExpanded);
  const hasChildren = Boolean(children);

  return (
    <div className="tree-node">
      <div
        className={`tree-node-row ${hasChildren ? 'clickable' : ''}`}
        onClick={() => hasChildren && setExpanded(!expanded)}
      >
        <span className="tree-toggle">
          {hasChildren ? (
            expanded ? <ChevronDown size={13} /> : <ChevronRight size={13} />
          ) : (
            <span className="tree-leaf-bullet" />
          )}
        </span>

        <span className="tree-label font-mono">{label}</span>
        {badge && <span className="tree-badge">{badge}</span>}
      </div>

      {hasChildren && expanded && (
        <div className="tree-children">
          {children}
        </div>
      )}
    </div>
  );
};

export const AstViewer: React.FC<AstViewerProps> = ({ ast }) => {
  const [viewMode, setViewMode] = useState<'visual' | 'json'>('visual');

  if (!ast) {
    return (
      <div className="empty-stage-state">
        <p>No AST available. Execute a valid SQL query to generate the syntax tree.</p>
      </div>
    );
  }

  const renderExpressionNode = (node: AstNodeJson | null | undefined): React.ReactNode => {
    if (!node) return null;

    if (node.type === 'BinaryOp') {
      return (
        <TreeNode label={`BinaryOp (${node.op})`} badge="Expression">
          {node.left && (
            <TreeNode label="Left Operand">
              {renderExpressionNode(node.left)}
            </TreeNode>
          )}
          {node.right && (
            <TreeNode label="Right Operand">
              {renderExpressionNode(node.right)}
            </TreeNode>
          )}
        </TreeNode>
      );
    }

    if (node.type === 'ColumnRef') {
      return (
        <TreeNode
          label={`ColumnRef: ${node.name}`}
          badge="Column"
        />
      );
    }

    if (node.type === 'Literal') {
      return (
        <TreeNode
          label={`Literal: ${node.value}`}
          badge={node.literalType}
        />
      );
    }

    return <TreeNode label={`${node.type}`} />;
  };

  return (
    <div className="ast-view">
      <div className="ast-toolbar">
        <div className="ast-toolbar-title">
          <span>Abstract Syntax Tree Representation</span>
        </div>
        <div className="view-mode-toggle">
          <button
            type="button"
            className={`mode-btn ${viewMode === 'visual' ? 'active' : ''}`}
            onClick={() => setViewMode('visual')}
          >
            Tree View
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
            {JSON.stringify(ast, null, 2)}
          </pre>
        </div>
      ) : (
        <div className="ast-tree-canvas">
          <TreeNode label={`SelectQueryNode`} badge="Root Query" defaultExpanded={true}>
            {/* Target Relation */}
            <TreeNode
              label={`Relation: ${ast.table || 'students'}`}
              badge="FROM"
            />

            {/* Projected Columns */}
            <TreeNode
              label={`Projection (${ast.columns?.length || 0} columns)`}
              badge="SELECT"
              defaultExpanded={true}
            >
              {ast.columns?.map((col, idx) => (
                <TreeNode
                  key={idx}
                  label={`Attribute: ${col}`}
                  badge="Field"
                />
              ))}
            </TreeNode>

            {/* Filter Condition */}
            {ast.where ? (
              <TreeNode
                label="Selection Predicate"
                badge="WHERE"
                defaultExpanded={true}
              >
                {renderExpressionNode(ast.where)}
              </TreeNode>
            ) : (
              <TreeNode label="Selection: None (Scan All)" badge="WHERE" />
            )}

            {/* Order By Clause */}
            {ast.orderBy ? (
              <TreeNode
                label={`Ordering Key: ${ast.orderBy} (${ast.orderAsc !== false ? 'ASC' : 'DESC'})`}
                badge="ORDER BY"
              />
            ) : (
              <TreeNode label="Ordering: None" badge="ORDER BY" />
            )}

            {/* Limit Clause */}
            {ast.limit !== undefined && ast.limit !== null ? (
              <TreeNode
                label={`Limit Count: ${ast.limit} tuples`}
                badge="LIMIT"
              />
            ) : (
              <TreeNode label="Limit: None (Unrestricted)" badge="LIMIT" />
            )}
          </TreeNode>
        </div>
      )}
    </div>
  );
};
