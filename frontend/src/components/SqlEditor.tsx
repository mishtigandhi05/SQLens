import React from 'react';
import { Play, Pause, StepForward, RotateCcw, FastForward, Code2 } from 'lucide-react';
import { QueryExample } from '../types/compiler';

interface SqlEditorProps {
  query: string;
  setQuery: (q: string) => void;
  examples: QueryExample[];
  onSelectExample: (example: QueryExample) => void;
  onRunQuery: () => void;
  onPauseToggle: () => void;
  onStepNext: () => void;
  onSkipToEnd: () => void;
  onReset: () => void;
  isProcessing: boolean;
  isPaused: boolean;
  currentStepIndex: number;
  totalSteps: number;
  speed: number;
  setSpeed: (s: number) => void;
  selectedExampleId: string;
}

export const SqlEditor: React.FC<SqlEditorProps> = ({
  query,
  setQuery,
  examples,
  onSelectExample,
  onRunQuery,
  onPauseToggle,
  onStepNext,
  onSkipToEnd,
  onReset,
  isProcessing,
  isPaused,
  currentStepIndex,
  totalSteps,
  speed,
  setSpeed,
  selectedExampleId,
}) => {
  const lineCount = query.split('\n').length;

  return (
    <div className="editor-card">
      {/* Editor Top Toolbar */}
      <div className="editor-toolbar">
        <div className="editor-toolbar-left">
          <Code2 size={15} className="editor-icon" />
          <span className="editor-label">SQL Query</span>
          <span className="editor-schema-hint">students (id, name, cgpa, age)</span>
        </div>

        <div className="editor-toolbar-right">
          <label className="example-select-label" htmlFor="sample-query-select">
            Example:
          </label>
          <select
            id="sample-query-select"
            className="example-select"
            value={selectedExampleId}
            onChange={(e) => {
              const ex = examples.find((x) => x.id === e.target.value);
              if (ex) onSelectExample(ex);
            }}
          >
            <option value="" disabled>Select query preset...</option>
            {examples.map((ex) => (
              <option key={ex.id} value={ex.id}>
                {ex.name}
              </option>
            ))}
          </select>
        </div>
      </div>

      {/* Editor Body */}
      <div className="editor-body">
        <div className="line-numbers" aria-hidden="true">
          {Array.from({ length: Math.max(lineCount, 4) }).map((_, i) => (
            <div key={i} className="line-no">{i + 1}</div>
          ))}
        </div>
        <textarea
          value={query}
          onChange={(e) => setQuery(e.target.value)}
          placeholder="Enter SQL statement (e.g., SELECT name, cgpa FROM students WHERE cgpa > 8.0;)"
          className="sql-textarea font-mono"
          rows={Math.max(lineCount, 4)}
          spellCheck={false}
        />
      </div>

      {/* Presets Row */}
      <div className="presets-bar">
        <span className="presets-label">Presets:</span>
        <div className="presets-list">
          {examples.map((ex) => (
            <button
              key={ex.id}
              type="button"
              onClick={() => onSelectExample(ex)}
              className={`preset-btn ${selectedExampleId === ex.id ? 'active' : ''}`}
              title={ex.description}
            >
              {ex.name.split(' (')[0]}
            </button>
          ))}
        </div>
      </div>

      {/* Controls Footer */}
      <div className="editor-footer">
        <div className="action-buttons-group">
          <button
            onClick={onRunQuery}
            disabled={isProcessing && !isPaused}
            className="btn btn-primary"
            type="button"
            title="Execute query and run animated compilation pipeline"
          >
            <Play size={14} fill="currentColor" />
            <span>{isProcessing && !isPaused ? 'Processing...' : 'Run Query'}</span>
          </button>

          <button
            onClick={onStepNext}
            className="btn btn-secondary"
            type="button"
            title="Step through the next compiler stage manually"
          >
            <StepForward size={14} />
            <span>Step Through</span>
          </button>

          {isProcessing && (
            <button
              onClick={onPauseToggle}
              className="btn btn-secondary"
              type="button"
              title={isPaused ? 'Resume animation' : 'Pause animation'}
            >
              {isPaused ? <Play size={13} fill="currentColor" /> : <Pause size={13} />}
              <span>{isPaused ? 'Resume' : 'Pause'}</span>
            </button>
          )}

          <button
            onClick={onSkipToEnd}
            disabled={!isProcessing}
            className="btn btn-secondary"
            type="button"
            title="Skip animation directly to final result"
          >
            <FastForward size={13} />
            <span>Skip</span>
          </button>

          <button
            onClick={onReset}
            className="btn btn-secondary"
            type="button"
            title="Reset compiler pipeline state"
          >
            <RotateCcw size={13} />
            <span>Reset</span>
          </button>
        </div>

        {/* Speed Controls */}
        <div className="speed-group">
          <span className="speed-label">Speed:</span>
          <div className="speed-segmented">
            <button
              type="button"
              className={`speed-option ${speed === 1500 ? 'active' : ''}`}
              onClick={() => setSpeed(1500)}
            >
              0.5x
            </button>
            <button
              type="button"
              className={`speed-option ${speed === 800 ? 'active' : ''}`}
              onClick={() => setSpeed(800)}
            >
              1.0x
            </button>
            <button
              type="button"
              className={`speed-option ${speed === 300 ? 'active' : ''}`}
              onClick={() => setSpeed(300)}
            >
              2.0x
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};
