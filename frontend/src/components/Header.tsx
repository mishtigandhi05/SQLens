import React from 'react';
import { BookOpen } from 'lucide-react';

interface HeaderProps {
  backendConnected: boolean;
  onOpenVivaModal: () => void;
}

export const Header: React.FC<HeaderProps> = ({ backendConnected, onOpenVivaModal }) => {
  return (
    <header className="header-container">
      <div className="header-left">
        <div className="brand-group">
          <span className="brand-name">SQLens</span>
          <span className="brand-separator">/</span>
          <span className="brand-subtitle">Compiler Design Laboratory</span>
        </div>
        <span className="brand-badge">MiniSQL Engine</span>
      </div>

      <div className="header-right">
        <div className="backend-status">
          <span className={`status-dot ${backendConnected ? 'connected' : 'disconnected'}`} />
          <span className="status-text">
            {backendConnected ? 'Backend Connected' : 'Backend Offline'}
          </span>
        </div>

        <button
          onClick={onOpenVivaModal}
          className="btn-header"
          type="button"
          title="Compiler concepts, viva questions, and architecture overview"
        >
          <BookOpen size={14} />
          <span>Viva Guide</span>
        </button>
      </div>
    </header>
  );
};
