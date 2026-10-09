import React from 'react';
import { ExecutionResultJson, CompilerPerformance } from '../types/compiler';

interface ResultTableProps {
  result: ExecutionResultJson | null | undefined;
  performance?: CompilerPerformance;
}

export const ResultTable: React.FC<ResultTableProps> = ({ result, performance }) => {
  if (!result || !result.success) {
    return (
      <div className="empty-stage-state">
        <p>No query results to display. Run a valid SQL query to view output records.</p>
      </div>
    );
  }

  const { columns, rows, rowCount } = result;

  return (
    <div className="result-view">
      <div className="result-meta-bar">
        <div className="meta-item">
          <span className="meta-label">Rows returned:</span>
          <span className="meta-value">{rowCount}</span>
        </div>

        <div className="meta-item">
          <span className="meta-label">Columns:</span>
          <span className="meta-value">{columns?.length || 0}</span>
        </div>

        {performance && (
          <div className="meta-item">
            <span className="meta-label">Execution Time:</span>
            <span className="meta-value font-mono">{performance.totalMs.toFixed(2)} ms</span>
          </div>
        )}
      </div>

      {rowCount === 0 ? (
        <div className="empty-result-note">
          <p>Query returned 0 rows. (No tuples matched the filter criteria or LIMIT was 0).</p>
        </div>
      ) : (
        <div className="table-container">
          <table className="clean-table result-data-table">
            <thead>
              <tr>
                <th style={{ width: '40px' }}>#</th>
                {columns?.map((col, idx) => (
                  <th key={idx}>{col}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {rows?.map((row, rowIdx) => (
                <tr key={rowIdx}>
                  <td className="text-muted cell-index">{rowIdx + 1}</td>
                  {columns?.map((_, colIdx) => (
                    <td key={colIdx} className="font-mono">
                      {row[colIdx] !== undefined ? String(row[colIdx]) : 'NULL'}
                    </td>
                  ))}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
};
