# SQLens

## An Educational SQL Compiler and Query Analysis Visualizer

SQLens is a lightweight educational SQL compiler designed to help
students understand how SQL queries pass through compiler phases.

The project demonstrates the relationship between Compiler Design
and Database Management Systems by exposing intermediate stages that
are normally hidden inside a database system.

## Compiler Pipeline

```text
SQL Query
    ↓
Lexical Analysis
    ↓
Syntax Analysis
    ↓
Symbol Table
    ↓
Semantic Analysis
    ↓
Abstract Syntax Tree
    ↓
Intermediate Representation
    ↓
Query Optimization
    ↓
Query Execution
    ↓
Result Visualization