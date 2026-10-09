-- ============================================================================
-- SQLens Test Queries Suite (Valid, Optimization, Edge, and Invalid Cases)
-- ============================================================================
-- Run any query directly using:
--   .\minisql.exe "<query>"
-- Or run a query stored in a file using:
--   .\minisql.exe test_queries.sql
-- ============================================================================

-- ============================================================================
-- 1. VALID QUERY CASES (Frontend + Backend Execution)
-- ============================================================================

-- Test 01: Simple Projection (Table Scan & Project)
-- SELECT name FROM students;

-- Test 02: Single Numeric Predicate Filter
-- SELECT name, cgpa FROM students WHERE cgpa > 8.0;

-- Test 03: Integer Comparison Filter
-- SELECT name, age FROM students WHERE age >= 20;

-- Test 04: Compound Predicate with Logical AND
-- SELECT name, cgpa FROM students WHERE cgpa > 8.0 AND age >= 20;

-- Test 05: Compound Predicate with Logical OR
-- SELECT name, cgpa, age FROM students WHERE cgpa > 9.0 OR age < 20;

-- Test 06: String Literal Equality Comparison
-- SELECT name FROM students WHERE name = 'Alice';

-- Test 07: PRIMARY REVIEW 2 DEMO QUERY (Projection Reordering / Late Projection)
-- Canonical plan places PROJECT before SORT, but cgpa is NOT in the SELECT list.
-- The optimizer defers PROJECT above SORT so SORT can access 'cgpa'.
-- SELECT name, age FROM students WHERE age >= 20 ORDER BY cgpa LIMIT 3;

-- Test 08: SAFE EARLY STOPPING TEST (LIMIT without ORDER BY)
-- Scan terminates once 2 qualifying rows are found without reading remaining rows.
-- SELECT name FROM students WHERE age >= 20 LIMIT 2;

-- Test 09: ORDER BY with Selected Column (Projection Reordering NOT REQUIRED)
-- Sort key 'cgpa' is already preserved in SELECT list, so PROJECT reordering is not needed.
-- SELECT name, cgpa FROM students WHERE cgpa > 7.5 ORDER BY cgpa LIMIT 3;

-- Test 10: LIMIT Larger Than Available Rows
-- Requesting LIMIT 10 on a 5-row table cleanly returns all 5 rows.
-- SELECT name FROM students LIMIT 10;

-- Test 11: Zero Row Limit (LIMIT 0)
-- Slices output to 0 rows and stops scan immediately.
-- SELECT name FROM students LIMIT 0;

-- Default active query executed when running ".\minisql.exe test_queries.sql":
SELECT name, age FROM students WHERE age >= 20 ORDER BY cgpa LIMIT 3;

-- ============================================================================
-- 2. INVALID QUERY CASES (Compiler Stage-Identified Error Handling)
-- ============================================================================

-- Test 12: Semantic Error - Unknown column in SELECT list
-- Responsible stage: [SEMANTIC ERROR]
-- Expected: Column 'salary' does not exist in table 'students'.
-- SELECT salary FROM students;

-- Test 13: Semantic Error - Unknown table in FROM clause
-- Responsible stage: [SEMANTIC ERROR]
-- Expected: Table 'teachers' does not exist.
-- SELECT name FROM teachers;

-- Test 14: Semantic Error - Unknown column in WHERE condition
-- Responsible stage: [SEMANTIC ERROR]
-- Expected: Column 'salary' does not exist in table 'students'.
-- SELECT name FROM students WHERE salary > 50000;

-- Test 15: Type Error - Incompatible types in comparison
-- Responsible stage: [TYPE ERROR]
-- Expected: Cannot compare FLOAT with STRING.
-- SELECT name FROM students WHERE cgpa > 'Alice';

-- Test 16: Syntax Error - Missing column list
-- Responsible stage: [SYNTAX ERROR]
-- Expected: Unexpected token FROM. Expected column identifier.
-- SELECT FROM students;

-- Test 17: Lexical Error - Invalid unrecognized character
-- Responsible stage: [LEXICAL ERROR]
-- Expected: Invalid character '@' at line 1.
-- SELECT @name FROM students;
