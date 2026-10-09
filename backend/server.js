const express = require('express');
const cors = require('cors');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = process.env.PORT || 3001;

// Project root where minisql.exe and students.csv reside
const PROJECT_ROOT = path.resolve(__dirname, '..');
const COMPILER_EXE = path.join(PROJECT_ROOT, 'minisql.exe');

app.use(cors());
app.use(express.json());

// Preset queries for interactive demonstration
const EXAMPLES = [
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

// Schema definition matching symbol_table.cpp
const SCHEMA = {
  tables: [
    {
      name: 'students',
      columns: [
        { name: 'id', type: 'INT', description: 'Primary student identifier' },
        { name: 'name', type: 'STRING', description: 'Student full name' },
        { name: 'cgpa', type: 'FLOAT', description: 'Cumulative Grade Point Average' },
        { name: 'age', type: 'INT', description: 'Student age in years' }
      ]
    }
  ]
};

// Health check endpoint
app.get('/api/health', (req, res) => {
  const compilerExists = fs.existsSync(COMPILER_EXE);
  res.json({
    status: 'ok',
    compiler: 'SQLens C++ Compiler Engine',
    version: '1.0.0',
    compilerExists,
    compilerPath: COMPILER_EXE
  });
});

// Examples library endpoint
app.get('/api/examples', (req, res) => {
  res.json(EXAMPLES);
});

// Schema metadata endpoint
app.get('/api/schema', (req, res) => {
  res.json(SCHEMA);
});

// Real compiler execution endpoint
app.post('/api/compile', (req, res) => {
  const { query } = req.body;

  if (!query || typeof query !== 'string' || query.trim() === '') {
    return res.status(400).json({
      success: false,
      failedStage: 'SOURCE',
      error: {
        stage: 'Source Input',
        category: 'INPUT ERROR',
        message: 'Empty query submitted. Please provide an SQL statement.'
      }
    });
  }

  if (!fs.existsSync(COMPILER_EXE)) {
    return res.status(500).json({
      success: false,
      failedStage: 'SYSTEM',
      error: {
        stage: 'Compiler Bridge',
        category: 'SYSTEM ERROR',
        message: `Compiler binary not found at ${COMPILER_EXE}. Please run build.ps1 first.`
      }
    });
  }

  // Execute actual C++ minisql.exe with --json flag
  execFile(
    COMPILER_EXE,
    ['--json', query.trim()],
    { cwd: PROJECT_ROOT, maxBuffer: 10 * 1024 * 1024 },
    (error, stdout, stderr) => {
      // minisql.exe outputs JSON to stdout whether successful or failing at any compiler stage
      const trimmedOutput = (stdout || '').trim();

      if (!trimmedOutput) {
        return res.status(500).json({
          success: false,
          failedStage: 'COMPILER_EXECUTION',
          error: {
            stage: 'Compiler Execution',
            category: 'INTERNAL ERROR',
            message: stderr || error?.message || 'Compiler produced no output.'
          }
        });
      }

      try {
        const parsedResult = JSON.parse(trimmedOutput);
        return res.json(parsedResult);
      } catch (parseError) {
        // Fallback if stdout wasn't pure JSON for any reason
        return res.status(500).json({
          success: false,
          failedStage: 'BRIDGE_SERIALIZATION',
          error: {
            stage: 'JSON Bridge',
            category: 'SERIALIZATION ERROR',
            message: 'Unable to parse compiler JSON output.',
            rawOutput: trimmedOutput
          }
        });
      }
    }
  );
});

app.listen(PORT, () => {
  console.log(`========================================`);
  console.log(` SQLens Backend API Bridge Server`);
  console.log(` Listening on http://localhost:${PORT}`);
  console.log(` Compiler: ${COMPILER_EXE}`);
  console.log(`========================================`);
});
