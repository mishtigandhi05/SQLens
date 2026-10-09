# SQLens: An Educational SQL Compiler and Query Analysis Visualizer

**SQLens** is an educational SQL compiler, relational optimizer, and query analysis visualizer written in C++ (Flex, Bison) with a modern web visualizer built on React, TypeScript, Vite, and Node.js. It bridges **Compiler Design** and **Database Management Systems (DBMS)** by turning the internal compilation, optimization, and execution pipeline of a relational query engine into an observable, animated, and interactive educational experience.

---

## 1. Project Overview & Objectives

### Problem Statement
In traditional Computer Science curricula, compiler courses often focus on programming languages (C, Java, Python), leaving database query compilers as abstract black boxes. Meanwhile, students studying DBMS learn relational algebra, query optimization trees, and physical plans without seeing how lexical, syntactic, semantic, and intermediate representations translate raw SQL into executable relational plans.

### Objectives
1. **End-to-End Transparency**: Expose all 10 stages of query compilation from source SQL to tabular results.
2. **Real C++ Backend**: Ensure zero fake/mocked compiler outputs—all tokens, AST trees, schemas, relational plans, rule explanations, and execution traces are generated directly by the C++ engine.
3. **Interactive Visualizer**: Provide an animated, step-by-step web workbench for classroom demonstrations and lab viva examinations.
4. **Strict Error Isolation**: Implement fail-stop error detection that halts at the exact stage of failure (lexical, syntax, semantic, type, or execution).

---

## 2. System Architecture

```text
                  ┌──────────────────────────────────────────────┐
                  │             SQLens Web Dashboard             │
                  │             React 18 + TypeScript            │
                  │         Vite Dev Server (Port 5173)          │
                  └──────────────────────┬───────────────────────┘
                                         │ HTTP POST /api/compile
                                         ▼
                  ┌──────────────────────────────────────────────┐
                  │             Backend API Bridge               │
                  │             Node.js + Express.js             │
                  │             Port 3001 (CORS Enabled)         │
                  └──────────────────────┬───────────────────────┘
                                         │ ChildProcess: minisql.exe --json
                                         ▼
                  ┌──────────────────────────────────────────────┐
                  │           SQLens Core C++ Compiler           │
                  │           Flex (Lexer) + Bison (LALR)        │
                  │       AST • Symbol Table • Type Checker      │
                  │      Relational IR • Optimizer • Executor    │
                  └──────────────────────┬───────────────────────┘
                                         │ Streaming File I/O
                                         ▼
                  ┌──────────────────────────────────────────────┐
                  │               Physical Storage               │
                  │               data/students.csv              │
                  └──────────────────────────────────────────────┘
```

---

## 3. The 10-Stage Compiler Pipeline

Every SQL query progresses through 10 stages:

```text
[01 Source SQL]
      ↓
[02 Lexical Analysis]       → Flex DFA scanner tokenizes input into TokenRecord stream
      ↓
[03 Syntax Analysis]        → Bison LALR(1) parser validates SQL grammar & operator precedence
      ↓
[04 Abstract Syntax Tree]   → Hierarchical AST (SelectQueryNode, BinaryOpNode, ColumnRef, Literal)
      ↓
[05 Symbol Table Lookup]    → Schema lookup (`students`: id INT, name STRING, cgpa FLOAT, age INT)
      ↓
[06 Semantic Analysis]      → Table & column existence verification and strict type checking
      ↓
[07 Relational IR]          → Canonical relational algebra plan (Scan, Filter, Project, Sort, Limit)
      ↓
[08 Query Optimization]     → Rule-based algebraic optimizer (Filter pushdown, Late projection, Limit)
      ↓
[09 Query Execution]        → In-memory CSV streaming execution engine with step-by-step trace
      ↓
[10 Query Result]           → Formatted answer set with row/column counts and high-res timing
```

---

## 4. Supported MiniSQL Language

SQLens compiles and executes the following declarative SQL subset:

* **Keywords**: `SELECT`, `FROM`, `WHERE`, `ORDER`, `BY`, `ASC`, `DESC`, `LIMIT`, `AND`, `OR`
* **Projection**: Single column, multiple comma-separated columns
* **Target Relations**: `students`
* **Comparisons**: `=`, `!=`, `<`, `>`, `<=`, `>=`
* **Logical Predicates**: `AND`, `OR`, and parenthesized sub-expressions `(condition)`
* **Ordering**: `ORDER BY <column> [ASC | DESC]` (including ordering on non-projected columns)
* **Row Restriction**: `LIMIT <integer>`
* **Literals**: Integers (`20`), Floating-point (`8.5`), Strings (`'Alice'`)
* **Comments**: Single-line SQL comments (`-- comment`)

### Database Schema (`data/students.csv`)
| Column | Data Type | Description |
|:-------|:----------|:------------|
| `id` | `INT` | Student identifier |
| `name` | `STRING` | Student full name |
| `cgpa` | `FLOAT` | Cumulative GPA (0.0 to 10.0) |
| `age` | `INT` | Student age in years |

---

## 5. Phase Implementation Status

### Phase 1: Problem Definition & Frontend Compiler (100% COMPLETE)
* [x] **Lexical Scanner (`lexer.l`)**: Flex tokenization of SQL keywords, symbols, literals, parentheses, comments, and invalid character trapping with column tracking.
* [x] **Syntax Parser (`parser.y`)**: Bison LALR(1) grammar for projection, filtering, boolean conjunctions/disjunctions, sorting, and limit clauses.
* [x] **Abstract Syntax Tree (`ast.h`, `ast.cpp`)**: Full AST node hierarchy with ASCII visualization and JSON serialization.
* [x] **Symbol Table (`symbol_table.h`, `symbol_table.cpp`)**: Schema catalog with column type lookup and JSON export.
* [x] **Semantic Analyzer (`semantic_analyzer.h`, `semantic_analyzer.cpp`)**: Validation of relation existence, attribute existence, and type compatibility.

### Phase 2: Core Relational Backend (100% COMPLETE)
* [x] **Relational IR (`ir.h`, `ir.cpp`)**: Relational algebra operator tree (`ScanNode`, `FilterNode`, `ProjectNode`, `SortNode`, `LimitNode`).
* [x] **Query Optimizer (`optimizer.h`, `optimizer.cpp`)**:
  1. *Filter Pushdown*: Evaluates predicates directly above scans to prune tuples early.
  2. *Projection Reordering / Late Projection*: Ensures columns needed for `WHERE` or `ORDER BY` are preserved before slicing.
  3. *Limit Optimization*: Enables streaming early stopping when ordering is absent.
* [x] **Query Execution Engine (`executor.h`, `executor.cpp`)**: CSV file parser, typed row evaluator, expression evaluation, in-memory sorting, projection, and early-stopping limits.
* [x] **Output & Tracing (`executor.cpp`, `main.cpp`)**: Tabular formatting with dynamic column widths and physical step execution traces.
* [x] **Fail-Stop Error Handling**: Stage-specific error reporting (`[LEXICAL ERROR]`, `[SYNTAX ERROR]`, `[SEMANTIC ERROR]`, `[TYPE ERROR]`, `[EXECUTION ERROR]`).

### Phase 3: Web Visualization, Integration & Testing (~65% ADVANCED PROGRESS)
* [x] **Machine-Readable CLI Bridge**: Added `--json` flag to `main.cpp` outputting pure JSON with nanosecond stage timings without breaking existing terminal output.
* [x] **Node/Express API Bridge (`backend/server.js`)**: Express server exposing `/api/compile`, `/api/health`, `/api/examples`, and `/api/schema`.
* [x] **Interactive Web Visualizer (`frontend/`)**: React 18, TypeScript, and Vite dashboard featuring:
  - 10-stage animated pipeline with Play, Pause, Step Through, Skip, and Reset controls.
  - Interactive AST collapsible tree visualization.
  - Relational IR tree and side-by-side **Original vs Optimized Plan** comparison.
  - Live execution trace and responsive result tables.
  - Stage Inspector modal providing compiler theory, input, output, and concepts for viva defense.
  - Viva Guide modal with quick reference compiler questions and answers.
* [x] **Automated Test Suite (`run_tests.ps1`)**: 26 automated tests covering unit features, edge cases, invalid inputs, and JSON bridge verification (**26/26 Passing**).

---

## 6. Optimization Transformations Explained

The SQLens rule-based optimizer implements algebraic rewrites:

### 1. Filter Pushdown
* **Rule**: $\sigma_{p}(\pi_{A}(R)) \rightarrow \pi_{A}(\sigma_{p}(R))$
* **Benefit**: Reduces the number of intermediate tuples before costly operations like sorting or network transfer.
* **Truthful Status**: If the filter is already directly above scan, the optimizer states `NOT REQUIRED` rather than falsely claiming an optimization.

### 2. Late Projection (Projection Reordering)
* **Rule**: When `ORDER BY col` references a column not in `SELECT col1, col2`, projection cannot occur before sort. The optimizer ensures sort has access to the sort key, then applies projection afterward.
* **Benefit**: Prevents query crashes and preserves correctness while minimizing tuple width.

### 3. Limit Early Stopping
* **Rule**: When no `ORDER BY` is present, `LIMIT N` is pushed directly into `ScanNode`, streaming only $N$ rows and stopping I/O immediately.
* **Benefit**: $O(N)$ row loading instead of scanning entire $M$-row tables.

---

## 7. Automated Test Suite (26/26 Tests)

Run the test suite using PowerShell:
```powershell
.\run_tests.ps1
```

### Test Coverage Summary:
| Category | Test IDs | Description | Status |
|:---------|:---------|:------------|:-------|
| **Valid Projections** | T01, T02 | Single and multi-column projections | PASS |
| **Predicate Filters** | T03, T04 | Float comparisons (`cgpa > 8.0`), Integer comparisons (`age >= 20`) | PASS |
| **Logical Predicates** | T05, T06 | Conjunctions (`AND`), Disjunctions (`OR`) | PASS |
| **Ordering & Limits** | T07, T08, T09 | `ORDER BY`, `LIMIT`, combined `ORDER BY + LIMIT` | PASS |
| **Compound Queries** | T10, T11 | `WHERE + ORDER BY + LIMIT`, order by unselected column | PASS |
| **Boundary Cases** | T12, T13, T14 | `LIMIT` without sort, `LIMIT 0`, `LIMIT > total rows` | PASS |
| **Strings & Parens** | T15, T16 | String equality (`name = 'Alice'`), compound boolean with parens & `DESC` | PASS |
| **Compiler Errors** | T17, T18 | Semantic errors (unknown column `salary`, unknown table `professors`) | PASS |
| **Type Errors** | T19 | Semantic type mismatch (`age > 'twenty'`) | PASS |
| **Syntax Errors** | T20, T22 | Incomplete queries (`SELECT FROM students;`), malformed clauses | PASS |
| **Lexical Errors** | T21 | Invalid characters (`SELECT @name FROM students;`) | PASS |
| **Execution Errors** | T23, T24, T25 | Missing CSV file, malformed CSV header, non-numeric data | PASS |
| **JSON API Bridge** | T26 | Machine-readable `--json` output validation | PASS |

```text
========================================
Test Execution Summary
========================================
TOTAL TESTS: 26
PASSED:      26
FAILED:      0
ALL TESTS PASSED WITH 100% SUCCESS RATE!
========================================
```

---

## 8. Installation & Setup

### Prerequisites
* **C++ Compiler**: MinGW-w64 (`g++`) with C++17 support.
* **Parser Generators**: Flex and Bison (`win_flex`, `win_bison` or MSYS2).
* **Node.js**: v18 or later.
* **Operating System**: Windows (PowerShell) / Linux / macOS.

### Step 1: Build the C++ Compiler
```powershell
# Compile lexer.l, parser.y, and C++ source files into minisql.exe
.\build.ps1
```

### Step 2: Install Backend Dependencies
```powershell
cd backend
npm install
cd ..
```

### Step 3: Install Frontend Dependencies
```powershell
cd frontend
npm install
cd ..
```

---

## 9. Running the Application

### Option A: Interactive Web Visualizer (Recommended for Demonstration)
1. **Start Backend API Bridge** (in a terminal):
   ```powershell
   cd backend
   node server.js
   ```
   *Runs at `http://localhost:3001`.*

2. **Start Frontend Visualizer** (in a second terminal):
   ```powershell
   cd frontend
   npm run dev
   ```
   *Open browser at **`http://localhost:5173`**.*

### Option B: Terminal CLI (Viva & Grading Mode)
You can directly run SQL queries in the terminal:
```powershell
# Interactive prompt
.\minisql.exe

# Execute a query directly
.\minisql.exe "SELECT name, cgpa FROM students WHERE cgpa > 8.0 ORDER BY cgpa LIMIT 3;"

# Execute query from file
.\minisql.exe input.sql

# Output pure structured JSON for programmatic consumption
.\minisql.exe --json "SELECT name, age FROM students WHERE age >= 20;"
```

---

## 10. Recommended Demonstration Queries for Review 2 / Viva

### Query 1: The Grand Tour (Demonstrates All 10 Stages + Late Projection)
```sql
SELECT name, age
FROM students
WHERE age >= 20
ORDER BY cgpa
LIMIT 3;
```
* **What to highlight**:
  1. Lexer breaks input into 14 distinct tokens.
  2. AST displays hierarchical tree with projection, filter, sort, and limit nodes.
  3. Symbol table verifies types (`name: STRING`, `age: INT`, `cgpa: FLOAT`).
  4. IR shows initial canonical plan with late projection needed because `cgpa` is sorted but not selected.
  5. Optimizer applies Late Projection and retains `cgpa` until sorting completes.
  6. CSV engine logs row-by-row filtering and returns top 3 students.

### Query 2: Semantic Fail-Stop Error Demonstration
```sql
SELECT salary FROM students;
```
* **What to highlight**:
  1. Lexer and Parser succeed.
  2. Pipeline halts immediately at **Stage 06 (Semantic Analysis)**.
  3. Displays: `Column 'salary' does not exist in table 'students'`.
  4. Stages 07–10 are never executed, proving strict fail-stop compiler semantics.

### Query 3: Lexical Error Demonstration
```sql
SELECT @name FROM students;
```
* **What to highlight**:
  1. Pipeline halts at **Stage 02 (Lexical Analysis)**.
  2. Displays: `Unrecognized character '@' at line 1, column 8`.
  3. Demonstrates compiler front-end error isolation.

---

## 11. Known Limitations & Scope Boundaries
* **Single Table Scope**: Supports relations over `students.csv`. Joins (`INNER JOIN`, `CROSS JOIN`) are part of optional Phase 3 extensions.
* **Aggregation Scope**: `GROUP BY`, `COUNT()`, `AVG()` are not in the core MiniSQL specification.
* **DML Scope**: Focus is on query compilation (`SELECT`). `INSERT`, `UPDATE`, `DELETE` are intentionally omitted.

---

## 12. Final 10% Roadmap (Reserved for Student Completion)
1. **Lab Report Formatting**: Export screenshots of the visualizer into the official university lab record template.
2. **Viva Oral Preparation**: Review the interactive *Viva & Theory Guide* modal inside the web app for oral defense questions.
3. **Optional Phase 3 Extensions**: Join algorithms (Nested Loop Join), `GROUP BY` aggregation, or indexing simulations if extra credit is desired.