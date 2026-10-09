import React from 'react';
import { StageDefinition, CompilerResponse } from '../types/compiler';
import { TokensTable } from './TokensTable';
import { AstViewer } from './AstViewer';
import { SemanticChecklistView } from './SemanticChecklistView';
import { IrViewer } from './IrViewer';
import { OptimizationViewer } from './OptimizationViewer';
import { ExecutionTraceView } from './ExecutionTraceView';
import { ResultTable } from './ResultTable';
import { ArrowRight, Check, AlertCircle } from 'lucide-react';

interface StageInspectorProps {
  stage: StageDefinition;
  compilerResult: CompilerResponse | null;
  query: string;
}

export const StageInspector: React.FC<StageInspectorProps> = ({
  stage,
  compilerResult,
  query,
}) => {
  const stagesData = compilerResult?.stages;

  const renderStageContent = () => {
    switch (stage.key) {
      case 'source':
        return (
          <div className="source-inspector-box">
            <div className="source-query-display font-mono">
              {query || 'SELECT name, cgpa FROM students WHERE cgpa > 8.0;'}
            </div>
            <div className="source-stats-row">
              <span className="stat-pill">Characters: {query.length}</span>
              <span className="stat-pill">Lines: {query.split('\n').length}</span>
              <span className="stat-pill">Grammar: MiniSQL</span>
            </div>
          </div>
        );

      case 'lexer':
        return <TokensTable tokens={stagesData?.lexer?.tokens || []} />;

      case 'parser':
        return (
          <div className="parser-view">
            {stagesData?.parser?.success === false ? (
              <div className="status-banner banner-error">
                <div className="banner-icon">
                  <AlertCircle size={16} strokeWidth={2.5} className="text-error" />
                </div>
                <div className="banner-body">
                  <div className="banner-title">Syntax Analysis Failed</div>
                  <div className="banner-description">
                    {stagesData.parser.error || compilerResult?.error?.message}
                  </div>
                </div>
              </div>
            ) : (
              <div className="status-banner banner-success">
                <div className="banner-icon">
                  <Check size={16} strokeWidth={2.5} className="text-success" />
                </div>
                <div className="banner-body">
                  <div className="banner-title">Syntax Analysis Passed</div>
                  <div className="banner-description">
                    Input tokens satisfied MiniSQL context-free grammar. Shift-reduce parsing completed without errors.
                  </div>
                </div>
              </div>
            )}

            <div className="grammar-rules-box">
              <div className="rules-heading">MiniSQL Production Rules (Bison LALR)</div>
              <ul className="rules-list font-mono">
                <li>query → SELECT column_list FROM identifier [opt_where] [opt_order_by] [opt_limit] ';'</li>
                <li>column_list → identifier | column_list ',' identifier</li>
                <li>condition → condition AND condition | condition OR condition | '(' condition ')' | expr comp_op expr</li>
                <li>comp_op → '=' | '!=' | '&lt;' | '&gt;' | '&lt;=' | '&gt;='</li>
                <li>opt_order_by → ε | ORDER BY identifier [ASC | DESC]</li>
                <li>opt_limit → ε | LIMIT integer</li>
              </ul>
            </div>
          </div>
        );

      case 'ast':
        return <AstViewer ast={stagesData?.parser?.ast || null} />;

      case 'symbolTable':
        return (
          <div className="symbol-table-view">
            <div className="schema-intro">
              <span className="schema-table-name">Relation: <strong>students</strong></span>
              <span className="schema-table-file text-muted font-mono">data/students.csv</span>
            </div>
            <div className="table-container">
              <table className="clean-table">
                <thead>
                  <tr>
                    <th>Column Attribute</th>
                    <th>Physical Data Type</th>
                    <th>Domain Description</th>
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    <td className="font-mono font-medium">id</td>
                    <td><span className="type-tag tag-literal">INT</span></td>
                    <td>Primary Student Identifier (Positive Integer)</td>
                  </tr>
                  <tr>
                    <td className="font-mono font-medium">name</td>
                    <td><span className="type-tag tag-keyword">STRING</span></td>
                    <td>Student Full Name (Text Literal)</td>
                  </tr>
                  <tr>
                    <td className="font-mono font-medium">cgpa</td>
                    <td><span className="type-tag tag-literal">FLOAT</span></td>
                    <td>Grade Point Average (Floating Point Real)</td>
                  </tr>
                  <tr>
                    <td className="font-mono font-medium">age</td>
                    <td><span className="type-tag tag-literal">INT</span></td>
                    <td>Student Age in Years (Integer)</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        );

      case 'semantic':
        return <SemanticChecklistView semantic={stagesData?.semantic} />;

      case 'ir':
        return <IrViewer plan={stagesData?.ir?.canonicalPlan} />;

      case 'optimizer':
        return <OptimizationViewer optimization={stagesData?.optimizer} />;

      case 'execution':
        return <ExecutionTraceView execution={stagesData?.execution} />;

      case 'result':
        return (
          <ResultTable
            result={stagesData?.result}
            performance={compilerResult?.performance}
          />
        );

      default:
        return null;
    }
  };

  return (
    <div className="inspector-card">
      {/* Inspector Top Meta */}
      <div className="inspector-header">
        <div className="inspector-meta-row">
          <div className="inspector-stage-label">
            <span className="stage-num-tag">Stage {stage.number}</span>
            <span className="stage-title-text">{stage.name}</span>
          </div>

          <div className="io-flow">
            <span className="io-item font-mono">{stage.inputDescription}</span>
            <ArrowRight size={12} className="io-arrow" />
            <span className="io-item font-mono">{stage.outputDescription}</span>
          </div>
        </div>

        <p className="inspector-desc">{stage.description}</p>
      </div>

      {/* Main Content */}
      <div className="inspector-content">
        {renderStageContent()}
      </div>
    </div>
  );
};
