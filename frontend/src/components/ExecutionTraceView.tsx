import React from 'react';
import { ExecutionResultJson } from '../types/compiler';
import { Check, AlertCircle } from 'lucide-react';

interface ExecutionTraceViewProps {
  execution: ExecutionResultJson | null | undefined;
}

export const ExecutionTraceView: React.FC<ExecutionTraceViewProps> = ({ execution }) => {
  if (!execution) {
    return (
      <div className="empty-stage-state">
        <p>No execution trace recorded. Execute a query to observe physical CSV runtime evaluation.</p>
      </div>
    );
  }

  const { success, errorMessage, trace, rowCount } = execution;

  return (
    <div className="execution-view">
      {/* Status banner */}
      <div className={`status-banner ${success ? 'banner-success' : 'banner-error'}`}>
        <div className="banner-icon">
          {success ? (
            <Check size={16} strokeWidth={2.5} className="text-success" />
          ) : (
            <AlertCircle size={16} strokeWidth={2.5} className="text-error" />
          )}
        </div>
        <div className="banner-body">
          <div className="banner-title">
            {success ? 'Execution Completed' : 'Execution Halted'}
          </div>
          <div className="banner-description">
            {success
              ? `Engine processed data/students.csv and emitted ${rowCount} qualifying row${rowCount === 1 ? '' : 's'}.`
              : errorMessage}
          </div>
        </div>
      </div>

      {/* Execution Step Log */}
      <div className="trace-card">
        <div className="trace-card-header">
          <span>Physical Operator Execution Trace</span>
          <span className="trace-count text-muted">{trace?.length || 0} steps</span>
        </div>

        <div className="trace-steps-container">
          {trace?.map((step, idx) => {
            const isEarlyStopping = step.toLowerCase().includes('early stopping');
            return (
              <div key={idx} className="trace-log-line">
                <span className="log-line-num font-mono">{idx + 1}</span>
                <span className="log-line-text font-mono">{step}</span>
                {isEarlyStopping && (
                  <span className="tag-early-stop">Early Stopped</span>
                )}
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
};
