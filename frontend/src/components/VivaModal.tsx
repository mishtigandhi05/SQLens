import React from 'react';
import { X } from 'lucide-react';

interface VivaModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export const VivaModal: React.FC<VivaModalProps> = ({ isOpen, onClose }) => {
  if (!isOpen) return null;

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <div className="modal-container" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <div className="modal-header-text">
            <h2 className="modal-title">SQLens Viva Guide & Compiler Lab Reference</h2>
            <p className="modal-subtitle">Compiler design theory, relational query processing, and architecture overview</p>
          </div>
          <button className="btn-modal-close" onClick={onClose} type="button">
            <X size={16} />
          </button>
        </div>

        <div className="modal-content">
          {/* Section 1: Pipeline Overview */}
          <section className="modal-block">
            <h3 className="block-title">1. Compiler Pipeline Architecture</h3>
            <p className="block-text">
              SQLens compiles and executes declarative SQL queries across a 10-stage pipeline:
            </p>
            <div className="arch-flow-box font-mono">
              SQL Query → Flex Lexer → Bison Parser → AST Node Tree → Symbol Table → Semantic Analyzer → Relational IR → Rule Optimizer → CSV Executor → Tabular Result
            </div>
          </section>

          {/* Section 2: Core Viva Questions */}
          <section className="modal-block">
            <h3 className="block-title">2. Key Viva Questions & Answers</h3>

            <div className="qa-pair">
              <h4 className="qa-question">Q1: How does Flex tokenize the SQL query and isolate lexical errors?</h4>
              <p className="qa-answer">
                Flex compiles regex rules into Deterministic Finite Automata (DFA). Keywords are matched case-insensitively, identifiers match <code>[a-zA-Z_][a-zA-Z0-9_]*</code>, literals match numeric and string patterns. Any unsupported character triggers the fallback rule <code>.</code> which flags a lexical error and halts the pipeline immediately.
              </p>
            </div>

            <div className="qa-pair">
              <h4 className="qa-question">Q2: How does Bison resolve operator precedence between AND and OR?</h4>
              <p className="qa-answer">
                Bison uses explicit precedence directives: <code>%left TOKEN_OR</code> followed by <code>%left TOKEN_AND</code> followed by comparison operators. This assigns <code>AND</code> higher precedence than <code>OR</code>, eliminating shift/reduce ambiguities in compound boolean WHERE clauses.
              </p>
            </div>

            <div className="qa-pair">
              <h4 className="qa-question">Q3: Why is Projection Reordering (Late Projection) necessary?</h4>
              <p className="qa-answer">
                In a naive canonical relational plan, <code>PROJECT</code> is positioned directly above <code>FILTER</code>, before <code>SORT</code>. If a query runs <code>SELECT name, age FROM students ORDER BY cgpa</code>, the <code>cgpa</code> column would be dropped by <code>PROJECT</code> before <code>SORT</code> can read it. The optimizer defers <code>PROJECT</code> until after <code>SORT</code>, preserving the sort key.
              </p>
            </div>

            <div className="qa-pair">
              <h4 className="qa-question">Q4: What is Filter Pushdown and when is it applied?</h4>
              <p className="qa-answer">
                Filter Pushdown pushes selection predicates ($\sigma$) as close to the leaf scan operator as possible. In SQLens, if a filter appears above a sort or project, it is pushed down adjacent to <code>ScanNode</code> to prune intermediate rows early. If the filter is already adjacent to scan, the optimizer truthfully reports <code>NOT REQUIRED</code>.
              </p>
            </div>

            <div className="qa-pair">
              <h4 className="qa-question">Q5: How does the CSV engine optimize LIMIT queries?</h4>
              <p className="qa-answer">
                When a query includes <code>LIMIT N</code> without an <code>ORDER BY</code> clause, the limit is pushed directly into the physical scan operator. The CSV reader stops scanning immediately after emitting $N$ qualifying rows, turning an $O(M)$ scan into an $O(N)$ early-stopping stream.
              </p>
            </div>
          </section>

          {/* Section 3: Schema Metadata */}
          <section className="modal-block">
            <h3 className="block-title">3. Catalog Schema (students.csv)</h3>
            <p className="block-text">
              The catalog symbol table declares four attributes:
              <code>id (INT)</code>, <code>name (STRING)</code>, <code>cgpa (FLOAT)</code>, and <code>age (INT)</code>.
            </p>
          </section>
        </div>
      </div>
    </div>
  );
};
