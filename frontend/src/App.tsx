import React, { useState, useEffect, useRef } from 'react';
import { Header } from './components/Header';
import { SqlEditor } from './components/SqlEditor';
import { PipelineVisualizer } from './components/PipelineVisualizer';
import { StageInspector } from './components/StageInspector';
import { VivaModal } from './components/VivaModal';
import { STAGES } from './constants/stages';
import {
  StageKey,
  StageStatus,
  CompilerResponse,
  QueryExample,
} from './types/compiler';
import { AlertCircle } from 'lucide-react';

const DEFAULT_QUERY = 'SELECT name, age FROM students WHERE age >= 20 ORDER BY cgpa LIMIT 3;';

const DEFAULT_EXAMPLES: QueryExample[] = [
  {
    id: 'demo-opt',
    name: 'Optimization Demo (Late Projection)',
    category: 'Optimization',
    description: 'Canonical plan has PROJECT before SORT, but cgpa is omitted from SELECT. Optimizer pushes PROJECT above SORT to preserve sort key.',
    query: 'SELECT name, age FROM students WHERE age >= 20 ORDER BY cgpa LIMIT 3;'
  },
  {
    id: 'demo-filter',
    name: 'Filtering (cgpa > 8.0)',
    category: 'Basic',
    description: 'Filters students with floating-point comparison and projects name and cgpa.',
    query: 'SELECT name, cgpa FROM students WHERE cgpa > 8.0;'
  },
  {
    id: 'demo-logical',
    name: 'Logical AND Operator',
    category: 'Logic',
    description: 'Evaluates compound condition combining FLOAT and INT comparisons with logical AND.',
    query: 'SELECT name, cgpa FROM students WHERE cgpa > 8.0 AND age >= 20;'
  },
  {
    id: 'demo-early-limit',
    name: 'Early Stopping (LIMIT without ORDER BY)',
    category: 'Optimization',
    description: 'CSV scanner stops immediately once 2 qualifying rows are emitted without reading subsequent records.',
    query: 'SELECT name FROM students WHERE age >= 20 LIMIT 2;'
  },
  {
    id: 'demo-complex',
    name: 'Complex Query (Parentheses + DESC + LIMIT)',
    category: 'Advanced',
    description: 'Evaluates parenthesized boolean predicates with descending sort and row limit.',
    query: 'SELECT id, name, cgpa, age FROM students WHERE (cgpa >= 8.0 AND age >= 20) OR id = 5 ORDER BY cgpa DESC LIMIT 4;'
  },
  {
    id: 'err-semantic',
    name: 'Semantic Error (Unknown Column)',
    category: 'Error',
    description: 'Halts compiler pipeline at Semantic Analysis when requesting a column not declared in schema.',
    query: 'SELECT salary FROM students;'
  },
  {
    id: 'err-type',
    name: 'Type Error (Incompatible Types)',
    category: 'Error',
    description: 'Halts compiler pipeline at Semantic Analysis due to comparing FLOAT with STRING literal.',
    query: "SELECT name FROM students WHERE cgpa > 'Alice';"
  },
  {
    id: 'err-syntax',
    name: 'Syntax Error (Missing Columns)',
    category: 'Error',
    description: 'Halts compiler pipeline at Bison Syntax Analysis due to unexpected FROM token.',
    query: 'SELECT FROM students;'
  },
  {
    id: 'err-lexical',
    name: 'Lexical Error (Invalid Character @)',
    category: 'Error',
    description: 'Halts compiler pipeline at Flex Lexical Analysis due to unsupported character.',
    query: 'SELECT @name FROM students;'
  }
];

export const App: React.FC = () => {
  const [query, setQuery] = useState(DEFAULT_QUERY);
  const [examples, setExamples] = useState<QueryExample[]>(DEFAULT_EXAMPLES);
  const [selectedExampleId, setSelectedExampleId] = useState('demo-opt');
  const [backendConnected, setBackendConnected] = useState(false);
  const [isVivaModalOpen, setIsVivaModalOpen] = useState(false);

  // Compiler results & pipeline state
  const [compilerResult, setCompilerResult] = useState<CompilerResponse | null>(null);
  const [activeStageKey, setActiveStageKey] = useState<StageKey>('source');
  const [stageStatuses, setStageStatuses] = useState<Record<StageKey, StageStatus>>({
    source: 'pending',
    lexer: 'pending',
    parser: 'pending',
    ast: 'pending',
    symbolTable: 'pending',
    semantic: 'pending',
    ir: 'pending',
    optimizer: 'pending',
    execution: 'pending',
    result: 'pending',
  });

  // Animation controls
  const [isProcessing, setIsProcessing] = useState(false);
  const [isPaused, setIsPaused] = useState(false);
  const [currentStepIndex, setCurrentStepIndex] = useState(0);
  const [speed, setSpeed] = useState(800);

  const animationTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const resultRef = useRef<CompilerResponse | null>(null);

  // Ping backend on mount
  useEffect(() => {
    fetch('/api/health')
      .then((res) => res.json())
      .then((data) => {
        if (data.status === 'ok') {
          setBackendConnected(true);
        }
      })
      .catch(() => setBackendConnected(false));

    // Load example presets from backend if available
    fetch('/api/examples')
      .then((res) => res.json())
      .then((data) => {
        if (Array.isArray(data) && data.length > 0) {
          setExamples(data);
        }
      })
      .catch(() => {
        // Keep default examples
      });
  }, []);

  const clearTimer = () => {
    if (animationTimerRef.current) {
      clearTimeout(animationTimerRef.current);
      animationTimerRef.current = null;
    }
  };

  const getStageKeyForIndex = (index: number): StageKey => {
    if (index >= 0 && index < STAGES.length) {
      return STAGES[index].key;
    }
    return 'source';
  };

  const mapFailedStageKey = (res: CompilerResponse): StageKey | null => {
    if (res.success || !res.failedStage) return null;
    switch (res.failedStage) {
      case 'LEXICAL_ANALYSIS':
        return 'lexer';
      case 'SYNTAX_ANALYSIS':
        return 'parser';
      case 'SEMANTIC_ANALYSIS':
        return 'semantic';
      case 'EXECUTION':
        return 'execution';
      default:
        return 'semantic';
    }
  };

  const runStep = (stepIdx: number, resultData: CompilerResponse) => {
    if (stepIdx >= STAGES.length) {
      setIsProcessing(false);
      setIsPaused(false);
      setActiveStageKey('result');
      return;
    }

    const currentKey = getStageKeyForIndex(stepIdx);
    setCurrentStepIndex(stepIdx);
    setActiveStageKey(currentKey);

    const failedKey = mapFailedStageKey(resultData);

    if (!resultData.success && failedKey === currentKey) {
      setStageStatuses((prev) => ({
        ...prev,
        [currentKey]: 'failed',
      }));
      setIsProcessing(false);
      setIsPaused(false);
      return;
    }

    setStageStatuses((prev) => ({
      ...prev,
      [currentKey]: 'success',
    }));

    if (stepIdx + 1 < STAGES.length) {
      const nextKey = getStageKeyForIndex(stepIdx + 1);
      setStageStatuses((prev) => ({
        ...prev,
        [nextKey]: 'processing',
      }));

      animationTimerRef.current = setTimeout(() => {
        runStep(stepIdx + 1, resultData);
      }, speed);
    } else {
      setIsProcessing(false);
      setIsPaused(false);
    }
  };

  // Run Query handler
  const handleRunQuery = async () => {
    clearTimer();
    setIsProcessing(true);
    setIsPaused(false);
    setCurrentStepIndex(0);
    setActiveStageKey('source');

    setStageStatuses({
      source: 'processing',
      lexer: 'pending',
      parser: 'pending',
      ast: 'pending',
      symbolTable: 'pending',
      semantic: 'pending',
      ir: 'pending',
      optimizer: 'pending',
      execution: 'pending',
      result: 'pending',
    });

    try {
      const response = await fetch('/api/compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ query: query.trim() }),
      });

      const data: CompilerResponse = await response.json();
      setCompilerResult(data);
      resultRef.current = data;

      animationTimerRef.current = setTimeout(() => {
        runStep(0, data);
      }, speed);
    } catch (err: any) {
      setIsProcessing(false);
      setStageStatuses((prev) => ({ ...prev, source: 'failed' }));
      const fallbackError: CompilerResponse = {
        success: false,
        query: query,
        failedStage: 'LEXICAL_ANALYSIS',
        error: {
          stage: 'BRIDGE',
          category: 'COMMUNICATION ERROR',
          message: err?.message || 'Could not connect to compiler API bridge.',
        },
      };
      setCompilerResult(fallbackError);
    }
  };

  // Step Next handler
  const handleStepNext = async () => {
    clearTimer();

    let res = resultRef.current;
    if (!res) {
      try {
        const response = await fetch('/api/compile', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ query: query.trim() }),
        });
        res = await response.json();
        setCompilerResult(res);
        resultRef.current = res;
      } catch (err) {
        return;
      }
    }

    if (!res) return;

    const failedKey = mapFailedStageKey(res);
    let nextIdx = currentStepIndex;

    if (!isProcessing && currentStepIndex === 0 && stageStatuses.source === 'pending') {
      nextIdx = 0;
    } else {
      nextIdx = currentStepIndex + 1;
    }

    if (nextIdx >= STAGES.length) return;

    const currentKey = getStageKeyForIndex(nextIdx);
    setCurrentStepIndex(nextIdx);
    setActiveStageKey(currentKey);

    if (!res.success && failedKey === currentKey) {
      setStageStatuses((prev) => ({
        ...prev,
        [currentKey]: 'failed',
      }));
      setIsProcessing(false);
      return;
    }

    setStageStatuses((prev) => ({
      ...prev,
      [currentKey]: 'success',
    }));
  };

  // Pause / Resume handler
  const handlePauseToggle = () => {
    if (!isProcessing) return;
    if (isPaused) {
      setIsPaused(false);
      if (resultRef.current) {
        runStep(currentStepIndex + 1, resultRef.current);
      }
    } else {
      clearTimer();
      setIsPaused(true);
    }
  };

  // Skip to end handler
  const handleSkipToEnd = () => {
    clearTimer();
    const res = resultRef.current;
    if (!res) return;

    const failedKey = mapFailedStageKey(res);
    const newStatuses: Record<StageKey, StageStatus> = { ...stageStatuses };

    let stopAtKey: StageKey | null = null;

    for (let i = 0; i < STAGES.length; ++i) {
      const k = STAGES[i].key;
      if (!res.success && failedKey === k) {
        newStatuses[k] = 'failed';
        stopAtKey = k;
        break;
      } else {
        newStatuses[k] = 'success';
      }
    }

    setStageStatuses(newStatuses);
    setIsProcessing(false);
    setIsPaused(false);

    if (stopAtKey) {
      setActiveStageKey(stopAtKey);
    } else {
      setActiveStageKey('result');
      setCurrentStepIndex(STAGES.length - 1);
    }
  };

  // Reset handler
  const handleReset = () => {
    clearTimer();
    setIsProcessing(false);
    setIsPaused(false);
    setCurrentStepIndex(0);
    setActiveStageKey('source');
    setCompilerResult(null);
    resultRef.current = null;
    setStageStatuses({
      source: 'pending',
      lexer: 'pending',
      parser: 'pending',
      ast: 'pending',
      symbolTable: 'pending',
      semantic: 'pending',
      ir: 'pending',
      optimizer: 'pending',
      execution: 'pending',
      result: 'pending',
    });
  };

  // Select example handler
  const handleSelectExample = (ex: QueryExample) => {
    setSelectedExampleId(ex.id);
    setQuery(ex.query);
    handleReset();
  };

  const activeStageDef = STAGES.find((s) => s.key === activeStageKey) || STAGES[0];

  return (
    <div className="app-wrapper">
      <Header
        backendConnected={backendConnected}
        onOpenVivaModal={() => setIsVivaModalOpen(true)}
      />

      <main className="content-container">
        {/* Section 1: Query Input & Controls */}
        <section className="section-block">
          <SqlEditor
            query={query}
            setQuery={setQuery}
            examples={examples}
            onSelectExample={handleSelectExample}
            onRunQuery={handleRunQuery}
            onPauseToggle={handlePauseToggle}
            onStepNext={handleStepNext}
            onSkipToEnd={handleSkipToEnd}
            onReset={handleReset}
            isProcessing={isProcessing}
            isPaused={isPaused}
            currentStepIndex={currentStepIndex}
            totalSteps={STAGES.length}
            speed={speed}
            setSpeed={setSpeed}
            selectedExampleId={selectedExampleId}
          />
        </section>

        {/* Section 2: Pipeline Stepper */}
        <section className="section-block">
          <PipelineVisualizer
            stageStatuses={stageStatuses}
            activeStageKey={activeStageKey}
            onSelectStage={(k) => setActiveStageKey(k)}
            currentStepIndex={currentStepIndex}
          />
        </section>

        {/* Global Error Banner if Stage Failed */}
        {compilerResult && !compilerResult.success && compilerResult.error && (
          <section className="section-block">
            <div className="error-alert-banner">
              <div className="error-alert-icon">
                <AlertCircle size={18} strokeWidth={2.5} />
              </div>
              <div className="error-alert-body">
                <div className="error-alert-header">
                  <span className="error-alert-stage">{compilerResult.error.stage}</span>
                  <span className="error-alert-category">{compilerResult.error.category}</span>
                </div>
                <div className="error-alert-message">{compilerResult.error.message}</div>
                <div className="error-alert-subtext">
                  The query processing pipeline halted at this stage. Subsequent intermediate representation, optimization, and physical execution stages have been safely stopped.
                </div>
              </div>
            </div>
          </section>
        )}

        {/* Section 3: Stage Details / Inspector Output */}
        <section className="section-block">
          <StageInspector
            stage={activeStageDef}
            compilerResult={compilerResult}
            query={query}
          />
        </section>
      </main>

      <VivaModal
        isOpen={isVivaModalOpen}
        onClose={() => setIsVivaModalOpen(false)}
      />
    </div>
  );
};
