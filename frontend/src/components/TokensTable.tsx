import React, { useState } from 'react';
import { TokenInfo } from '../types/compiler';
import { Search } from 'lucide-react';

interface TokensTableProps {
  tokens: TokenInfo[];
}

export const TokensTable: React.FC<TokensTableProps> = ({ tokens }) => {
  const [searchTerm, setSearchTerm] = useState('');
  const [filterType, setFilterType] = useState('ALL');

  if (!tokens || tokens.length === 0) {
    return (
      <div className="empty-stage-state">
        <p>No tokens recorded. Execute a query to view lexical analysis output.</p>
      </div>
    );
  }

  const tokenTypes = Array.from(new Set(tokens.map((t) => t.type))).sort();

  const filteredTokens = tokens.filter((t) => {
    const matchesSearch =
      t.token.toLowerCase().includes(searchTerm.toLowerCase()) ||
      t.lexeme.toLowerCase().includes(searchTerm.toLowerCase());
    const matchesType = filterType === 'ALL' || t.type === filterType;
    return matchesSearch && matchesType;
  });

  const getTypeBadgeClass = (type: string) => {
    if (type.includes('KEYWORD')) return 'tag-keyword';
    if (type.includes('IDENTIFIER')) return 'tag-identifier';
    if (type.includes('LITERAL')) return 'tag-literal';
    if (type.includes('OPERATOR') || ['EQUALS', 'NOT_EQUALS', 'GREATER_THAN', 'LESS_THAN', 'GREATER_EQUAL', 'LESS_EQUAL'].includes(type)) return 'tag-operator';
    return 'tag-default';
  };

  return (
    <div className="tokens-view">
      <div className="table-filter-bar">
        <div className="search-box">
          <Search size={13} className="search-icon" />
          <input
            type="text"
            placeholder="Filter tokens or lexemes..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            className="filter-input"
          />
        </div>

        <div className="type-filter">
          <select
            value={filterType}
            onChange={(e) => setFilterType(e.target.value)}
            className="filter-select"
          >
            <option value="ALL">All Token Types ({tokens.length})</option>
            {tokenTypes.map((type) => (
              <option key={type} value={type}>
                {type}
              </option>
            ))}
          </select>
        </div>
      </div>

      <div className="table-container">
        <table className="clean-table">
          <thead>
            <tr>
              <th style={{ width: '40px' }}>#</th>
              <th>Lexeme</th>
              <th>Token Name</th>
              <th>Classification</th>
              <th style={{ width: '70px', textAlign: 'right' }}>Pos</th>
            </tr>
          </thead>
          <tbody>
            {filteredTokens.map((t, idx) => (
              <tr key={idx}>
                <td className="text-muted cell-index">{idx + 1}</td>
                <td className="cell-lexeme font-mono">{t.lexeme}</td>
                <td className="cell-token font-mono">{t.token}</td>
                <td>
                  <span className={`type-tag ${getTypeBadgeClass(t.type)}`}>
                    {t.type}
                  </span>
                </td>
                <td className="cell-pos text-muted font-mono" style={{ textAlign: 'right' }}>
                  {t.line}:{t.column}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      <div className="table-footer-summary">
        <span>Showing {filteredTokens.length} of {tokens.length} total tokens</span>
      </div>
    </div>
  );
};
