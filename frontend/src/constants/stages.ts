import { StageDefinition } from '../types/compiler';

export const STAGES: StageDefinition[] = [
  {
    key: 'source',
    number: 1,
    name: 'Source SQL',
    shortName: 'Source',
    description: 'Raw declarative query text input provided by the user or application.',
    compilerConcept: 'Declarative source language input conforming to Relational Calculus / SQL dialect grammar.',
    purpose: 'Serves as the high-level declarative input describing the desired result set without specifying physical evaluation procedures.',
    inputDescription: 'User-entered SQL text string.',
    outputDescription: 'Clean normalized SQL query buffer passed to the lexical analyzer.'
  },
  {
    key: 'lexer',
    number: 2,
    name: 'Lexical Analysis',
    shortName: 'Lexer',
    description: 'Scans raw character stream and emits discrete tokens using Flex deterministic finite automata (DFA).',
    compilerConcept: 'Lexical analysis translates regular expressions into DFA transition tables, identifying lexemes and categorizing them into tokens.',
    purpose: 'Converts unstructured characters into a structured token stream while stripping comments and whitespace and detecting lexical errors.',
    inputDescription: 'Raw query character stream.',
    outputDescription: 'Ordered sequence of lexical tokens (Keywords, Identifiers, Literals, Operators, Punctuation).'
  },
  {
    key: 'parser',
    number: 3,
    name: 'Syntax Analysis',
    shortName: 'Parser',
    description: 'Bison LALR(1) shift-reduce parser verifies grammatical structure against context-free grammar rules.',
    compilerConcept: 'Context-Free Grammar (CFG) validation with bottom-up LALR(1) parsing tables, shift/reduce conflict resolution, and operator precedence.',
    purpose: 'Ensures the query adheres strictly to the SQL grammar specification and constructs semantic action trees.',
    inputDescription: 'Token stream emitted by the Flex lexical analyzer.',
    outputDescription: 'Validated syntactic parse tree structure and derivation reduction.'
  },
  {
    key: 'ast',
    number: 4,
    name: 'Abstract Syntax Tree',
    shortName: 'AST',
    description: 'Hierarchical object-oriented representation of the syntactic structure, abstracting away concrete syntax tokens.',
    compilerConcept: 'Abstract Syntax Tree (AST) condenses concrete parse trees into semantic nodes representing operators, operands, clauses, and relations.',
    purpose: 'Provides a clean in-memory tree data structure for downstream traversal, semantic type inference, and plan generation.',
    inputDescription: 'Bison grammar semantic reductions.',
    outputDescription: 'Object-oriented AST root node (SelectQueryNode) containing column lists, FROM target, condition expressions, ORDER BY, and LIMIT.'
  },
  {
    key: 'symbolTable',
    number: 5,
    name: 'Symbol Table Lookup',
    shortName: 'Symbols',
    description: 'Consults database catalog schema mapping relations and attributes to declared physical data types.',
    compilerConcept: 'Symbol tables manage identifier bindings, scoping rules, and relational schemas (table names, column types, attribute nullability).',
    purpose: 'Provides the ground truth schema environment needed by the semantic analyzer to validate entity existence and inspect types.',
    inputDescription: 'AST relation names (e.g. table "students").',
    outputDescription: 'Catalog schema bindings for tables (id: INT, name: STRING, cgpa: FLOAT, age: INT).'
  },
  {
    key: 'semantic',
    number: 6,
    name: 'Semantic Analysis',
    shortName: 'Semantic',
    description: 'Performs entity verification, scope binding, and type checking across expressions and clauses.',
    compilerConcept: 'Static semantic verification, type inference rules, operator domain compatibility, and fail-stop diagnostic generation.',
    purpose: 'Catches non-syntactic semantic errors (unknown tables, invalid columns, illegal comparisons like FLOAT with STRING) before code generation.',
    inputDescription: 'AST combined with Symbol Table schema metadata.',
    outputDescription: 'Validated, type-checked query tree guaranteed to be semantically sound.'
  },
  {
    key: 'ir',
    number: 7,
    name: 'Intermediate Representation',
    shortName: 'Relational IR',
    description: 'Translates validated declarative AST into a canonical Relational Algebra operator tree (SCAN, FILTER, PROJECT, SORT, LIMIT).',
    compilerConcept: 'Relational Algebra (σ, π, τ, SCAN) operator trees bridge high-level declarative syntax with procedural runtime execution.',
    purpose: 'Decouples query language syntax from storage and execution engines, allowing algebraic transformations and optimizations.',
    inputDescription: 'Semantically validated AST.',
    outputDescription: 'Canonical Relational Algebra operator tree ready for algebraic optimization.'
  },
  {
    key: 'optimizer',
    number: 8,
    name: 'Query Optimization',
    shortName: 'Optimizer',
    description: 'Rule-based relational optimizer evaluates Filter Pushdown, Late Projection Reordering, and Limit Early Stopping.',
    compilerConcept: 'Rule-based algebraic equivalence transformations to minimize intermediate relation cardinalities and avoid premature column dropping.',
    purpose: 'Transforms canonical plan into an algebraically equivalent but computationally more efficient physical plan with full semantic preservation.',
    inputDescription: 'Canonical relational operator tree.',
    outputDescription: 'Optimized relational execution plan accompanied by detailed rule application reports.'
  },
  {
    key: 'execution',
    number: 9,
    name: 'Query Execution Engine',
    shortName: 'Executor',
    description: 'Runtime engine executes the relational plan over CSV storage with typed row evaluation and streaming early stopping.',
    compilerConcept: 'Volcano/Iterator dataflow execution model applying physical relational operators over in-memory tuples.',
    purpose: 'Loads raw records from CSV files, evaluates boolean predicate expressions, sorts tuples, projects columns, and slices limits.',
    inputDescription: 'Optimized relational IR plan and students.csv dataset.',
    outputDescription: 'Physical execution trace with intermediate tuple counts and filtered in-memory result table.'
  },
  {
    key: 'result',
    number: 10,
    name: 'Query Result',
    shortName: 'Result',
    description: 'Final tabular formatted output displaying returned rows, column headers, and execution metrics.',
    compilerConcept: 'Relational tuple projection and tabular presentation format.',
    purpose: 'Provides the end-user or client with the final computed query answer set alongside runtime performance metrics.',
    inputDescription: 'Output relation produced by the Query Execution Engine.',
    outputDescription: 'Structured tabular answer set, total row count, and compiler stage timing.'
  }
];
