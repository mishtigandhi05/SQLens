import React from 'react';
import { SemanticJson } from '../types/compiler';
import { Check, AlertCircle } from 'lucide-react';

interface SemanticChecklistViewProps {
  semantic: SemanticJson | null | undefined;
}

export const SemanticChecklistView: React.FC<SemanticChecklistViewProps> = ({ semantic }) => {
  if (!semantic) {
    return (
      <div className="empty-stage-state">
        <p>No semantic analysis report yet. Execute a query to verify schema semantics.</p>
      </div>
    );
  }

  const { success, category, message, checklist } = semantic;

  return (
    <div className="semantic-view">
      {/* Verification Status Banner */}
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
            {success ? 'Semantic Verification Passed' : `${category || 'Semantic Error'}`}
          </div>
          <div className="banner-description">
            {success
              ? 'All referenced relations, attributes, and expression types conform to the catalog schema.'
              : message}
          </div>
        </div>
      </div>

      {/* Checklist */}
      <div className="checklist-container">
        <div className="checklist-heading">Verification Checklist</div>
        <div className="checklist-items">
          {checklist?.map((item, idx) => (
            <div key={idx} className={`checklist-row ${item.passed ? 'is-passed' : 'is-failed'}`}>
              <div className="check-indicator">
                {item.passed ? (
                  <Check size={13} strokeWidth={2.5} className="text-success" />
                ) : (
                  <AlertCircle size={13} strokeWidth={2.5} className="text-error" />
                )}
              </div>
              <div className="check-main">
                <div className="check-top">
                  <span className="check-title">{item.check}</span>
                  {item.entity && (
                    <span className="check-tag font-mono">{item.entity}</span>
                  )}
                </div>
                <div className="check-detail">{item.message}</div>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};
