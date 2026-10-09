import React from 'react';
import { Check, AlertCircle, ChevronRight } from 'lucide-react';
import { STAGES } from '../constants/stages';
import { StageKey, StageStatus } from '../types/compiler';

interface PipelineVisualizerProps {
  stageStatuses: Record<StageKey, StageStatus>;
  activeStageKey: StageKey;
  onSelectStage: (key: StageKey) => void;
  currentStepIndex: number;
}

export const PipelineVisualizer: React.FC<PipelineVisualizerProps> = ({
  stageStatuses,
  activeStageKey,
  onSelectStage,
}) => {
  const completedCount = Object.values(stageStatuses).filter((s) => s === 'success').length;
  const isFailed = Object.values(stageStatuses).some((s) => s === 'failed');

  return (
    <div className="pipeline-card">
      <div className="pipeline-header">
        <div className="pipeline-header-left">
          <span className="pipeline-title">Compilation Pipeline</span>
          <span className="pipeline-subtitle">
            {isFailed
              ? 'Compilation halted on error'
              : `${completedCount} of ${STAGES.length} stages complete`}
          </span>
        </div>

        <div className="pipeline-legend">
          <span className="legend-item">
            <span className="legend-dot dot-pending" />
            <span>Pending</span>
          </span>
          <span className="legend-item">
            <span className="legend-dot dot-processing" />
            <span>Active</span>
          </span>
          <span className="legend-item">
            <span className="legend-dot dot-success" />
            <span>Passed</span>
          </span>
          <span className="legend-item">
            <span className="legend-dot dot-failed" />
            <span>Failed</span>
          </span>
        </div>
      </div>

      {/* Progress Track */}
      <div className="pipeline-timeline">
        {STAGES.map((stage, idx) => {
          const status = stageStatuses[stage.key] || 'pending';
          const isSelected = activeStageKey === stage.key;

          return (
            <React.Fragment key={stage.key}>
              <button
                type="button"
                onClick={() => onSelectStage(stage.key)}
                className={`timeline-node status-${status} ${isSelected ? 'selected' : ''}`}
                title={`Stage ${stage.number}: ${stage.name} (${status})`}
              >
                <div className="node-indicator">
                  {status === 'success' ? (
                    <Check size={12} strokeWidth={2.5} className="icon-success" />
                  ) : status === 'failed' ? (
                    <AlertCircle size={12} strokeWidth={2.5} className="icon-failed" />
                  ) : status === 'processing' ? (
                    <span className="node-spinner" />
                  ) : (
                    <span className="node-number">{stage.number}</span>
                  )}
                </div>

                <div className="node-label">
                  <span className="node-name">{stage.shortName}</span>
                </div>
              </button>

              {idx < STAGES.length - 1 && (
                <div className={`timeline-connector ${status === 'success' ? 'connector-passed' : ''}`}>
                  <ChevronRight size={12} className="connector-chevron" />
                </div>
              )}
            </React.Fragment>
          );
        })}
      </div>
    </div>
  );
};
