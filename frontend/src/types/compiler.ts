export type StageKey =
  | 'source'
  | 'lexer'
  | 'parser'
  | 'ast'
  | 'symbolTable'
  | 'semantic'
  | 'ir'
  | 'optimizer'
  | 'execution'
  | 'result';

export type StageStatus = 'pending' | 'processing' | 'success' | 'failed';

export interface StageDefinition {
  key: StageKey;
  number: number;
  name: string;
  shortName: string;
  description: string;
  compilerConcept: string;
  purpose: string;
  inputDescription: string;
  outputDescription: string;
}

export interface TokenInfo {
  token: string;
  lexeme: string;
  type: string;
  line: number;
  column?: number;
}

export interface AstNodeJson {
  type: string;
  table?: string;
  columns?: string[];
  where?: AstNodeJson | null;
  orderBy?: string | null;
  orderAsc?: boolean;
  limit?: number;
  op?: string;
  left?: AstNodeJson | null;
  right?: AstNodeJson | null;
  name?: string;
  value?: string;
  literalType?: string;
}

export interface SymbolTableColumn {
  name: string;
  type: string;
  description?: string;
}

export interface SymbolTableTable {
  name: string;
  columns: SymbolTableColumn[];
}

export interface SymbolTableJson {
  tables: SymbolTableTable[];
}

export interface SemanticCheckItem {
  check: string;
  entity: string;
  passed: boolean;
  message: string;
}

export interface SemanticJson {
  success: boolean;
  category: string;
  message: string;
  checklist: SemanticCheckItem[];
}

export interface IrNodeJson {
  op: string;
  details?: string;
  tableName?: string;
  columns?: string[];
  column?: string;
  ascending?: boolean;
  limit?: number;
  child?: IrNodeJson | null;
}

export interface RuleReport {
  ruleName: string;
  applied: boolean;
  status: 'APPLIED' | 'NOT REQUIRED' | 'NOT APPLICABLE';
  details: string;
}

export interface OptimizerJson {
  originalPlan: IrNodeJson;
  optimizedPlan: IrNodeJson;
  rules: RuleReport[];
}

export interface ExecutionResultJson {
  success: boolean;
  errorMessage: string;
  trace: string[];
  columns: string[];
  rows: (string | number)[][];
  rowCount: number;
}

export interface CompilerPerformance {
  lexerMs: number;
  parserMs: number;
  semanticMs: number;
  irMs: number;
  optimizerMs: number;
  executionMs: number;
  totalMs: number;
}

export interface CompilerStages {
  source?: { query: string };
  lexer?: { success: boolean; tokens: TokenInfo[] };
  parser?: { success: boolean; ast: AstNodeJson; error?: string };
  symbolTable?: SymbolTableJson;
  semantic?: SemanticJson;
  ir?: { canonicalPlan: IrNodeJson };
  optimizer?: OptimizerJson;
  execution?: ExecutionResultJson;
  result?: ExecutionResultJson;
}

export interface CompilerError {
  stage: string;
  category: string;
  message: string;
}

export interface CompilerResponse {
  success: boolean;
  query: string;
  failedStage?: string;
  error?: CompilerError;
  stages?: CompilerStages;
  performance?: CompilerPerformance;
}

export interface QueryExample {
  id: string;
  name: string;
  category: string;
  description: string;
  query: string;
}
