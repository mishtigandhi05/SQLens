# SQLens Automated Test Suite Runner
# Executes test queries across frontend, backend, optimization, CSV runtime, and error handling

$ErrorActionPreference = "Continue"

Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "                     SQLens COMPILER TEST SUITE                       " -ForegroundColor Cyan
Write-Host "======================================================================`n" -ForegroundColor Cyan

if (-not (Test-Path ".\minisql.exe")) {
    Write-Host "ERROR: minisql.exe not found! Run build.ps1 first." -ForegroundColor Red
    exit 1
}

$tests = @(
    # --- VALID TEST CASES (T1 - T16) ---
    @{
        Id = "T01"
        Name = "Simple SELECT"
        Query = "SELECT name FROM students;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 5"
    },
    @{
        Id = "T02"
        Name = "Multiple column projection"
        Query = "SELECT id, name, cgpa FROM students;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 5"
    },
    @{
        Id = "T03"
        Name = "Numeric filter (cgpa > 8.0)"
        Query = "SELECT name, cgpa FROM students WHERE cgpa > 8.0;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 3"
    },
    @{
        Id = "T04"
        Name = "Integer comparison (age >= 20)"
        Query = "SELECT name, age FROM students WHERE age >= 20;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 4"
    },
    @{
        Id = "T05"
        Name = "Logical AND condition"
        Query = "SELECT name, cgpa FROM students WHERE cgpa > 8.0 AND age >= 20;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 2"
    },
    @{
        Id = "T06"
        Name = "Logical OR condition"
        Query = "SELECT name, cgpa, age FROM students WHERE cgpa > 9.0 OR age < 20;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 2"
    },
    @{
        Id = "T07"
        Name = "ORDER BY sorting"
        Query = "SELECT name, cgpa FROM students ORDER BY cgpa;"
        ExpectExit = 0
        ExpectedPattern = "5 rows sorted"
    },
    @{
        Id = "T08"
        Name = "LIMIT slicing"
        Query = "SELECT name FROM students LIMIT 3;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 3"
    },
    @{
        Id = "T09"
        Name = "ORDER BY + LIMIT"
        Query = "SELECT name, cgpa FROM students ORDER BY cgpa LIMIT 3;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 3"
    },
    @{
        Id = "T10"
        Name = "WHERE + ORDER BY + LIMIT"
        Query = "SELECT name, cgpa FROM students WHERE cgpa > 7.5 ORDER BY cgpa LIMIT 3;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 3"
    },
    @{
        Id = "T11"
        Name = "ORDER BY non-selected column (Late Projection)"
        Query = "SELECT name, age FROM students WHERE age >= 20 ORDER BY cgpa LIMIT 3;"
        ExpectExit = 0
        ExpectedPattern = "Deferred PROJECT above SORT"
    },
    @{
        Id = "T12"
        Name = "LIMIT without ORDER BY (Early Stopping)"
        Query = "SELECT name FROM students WHERE age >= 20 LIMIT 2;"
        ExpectExit = 0
        ExpectedPattern = "early stopping applied"
    },
    @{
        Id = "T13"
        Name = "Zero row limit (LIMIT 0)"
        Query = "SELECT name FROM students LIMIT 0;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 0"
    },
    @{
        Id = "T14"
        Name = "LIMIT greater than row count"
        Query = "SELECT name FROM students LIMIT 10;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 5"
    },
    @{
        Id = "T15"
        Name = "String comparison (name = 'Alice')"
        Query = "SELECT name FROM students WHERE name = 'Alice';"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 1"
    },
    @{
        Id = "T16"
        Name = "Complex valid query (Compound boolean, DESC sort, LIMIT)"
        Query = "SELECT id, name, cgpa, age FROM students WHERE (cgpa >= 8.0 AND age >= 20) OR id = 5 ORDER BY cgpa DESC LIMIT 4;"
        ExpectExit = 0
        ExpectedPattern = "Rows returned: 4"
    },

    # --- INVALID TEST CASES (T17 - T22) ---
    @{
        Id = "T17"
        Name = "Unknown column in SELECT"
        Query = "SELECT salary FROM students;"
        ExpectExit = 3
        ExpectedPattern = "\[SEMANTIC ERROR\]"
    },
    @{
        Id = "T18"
        Name = "Unknown table in FROM"
        Query = "SELECT name FROM teachers;"
        ExpectExit = 3
        ExpectedPattern = "\[SEMANTIC ERROR\]"
    },
    @{
        Id = "T19"
        Name = "Type mismatch in WHERE comparison"
        Query = "SELECT name FROM students WHERE cgpa > 'Alice';"
        ExpectExit = 3
        ExpectedPattern = "\[TYPE ERROR\]"
    },
    @{
        Id = "T20"
        Name = "Invalid syntax (Missing column list)"
        Query = "SELECT FROM students;"
        ExpectExit = 2
        ExpectedPattern = "\[SYNTAX ERROR\]"
    },
    @{
        Id = "T21"
        Name = "Invalid lexical character (@)"
        Query = "SELECT @name FROM students;"
        ExpectExit = 1
        ExpectedPattern = "\[LEXICAL ERROR\]"
    },
    @{
        Id = "T22"
        Name = "Malformed query (Missing FROM keyword)"
        Query = "SELECT name students;"
        ExpectExit = 2
        ExpectedPattern = "\[SYNTAX ERROR\]"
    }
)

$passedCount = 0
$failedCount = 0

foreach ($t in $tests) {
    $out = .\minisql.exe $t.Query 2>&1 | Out-String
    $actualExit = $LASTEXITCODE

    $exitOk = ($actualExit -eq $t.ExpectExit)
    $patternOk = ($out -match $t.ExpectedPattern)

    if ($exitOk -and $patternOk) {
        $passedCount++
        Write-Host "$($t.Id) | PASS | $($t.Name)" -ForegroundColor Green
    } else {
        $failedCount++
        Write-Host "$($t.Id) | FAIL | $($t.Name)" -ForegroundColor Red
        Write-Host "   Expected Exit: $($t.ExpectExit), Got: $actualExit" -ForegroundColor Yellow
        Write-Host "   Expected Pattern: $($t.ExpectedPattern)" -ForegroundColor Yellow
    }
}

# ======================================================================
# CSV Engine Robustness Tests (T23 - T25)
# ======================================================================
$origCsvContent = Get-Content "students.csv" -Raw

# T23: Missing Data Source / Empty CSV
"" | Out-File -Encoding ascii "students.csv"
$out23 = .\minisql.exe "SELECT name FROM students;" 2>&1 | Out-String
$exit23 = $LASTEXITCODE
if ($exit23 -eq 4 -and ($out23 -match "empty")) {
    $passedCount++
    Write-Host "T23 | PASS | Missing data source (Empty/missing CSV data)" -ForegroundColor Green
} else {
    $failedCount++
    Write-Host "T23 | FAIL | Missing data source (Empty/missing CSV data)" -ForegroundColor Red
}

# T24: Malformed CSV Row (Wrong column count)
"id,name,cgpa,age`n1,Alice,8.5,20`n2,Bob,7.8" | Out-File -Encoding ascii "students.csv"
$out24 = .\minisql.exe "SELECT name FROM students;" 2>&1 | Out-String
$exit24 = $LASTEXITCODE
if ($exit24 -eq 4 -and ($out24 -match "Malformed row")) {
    $passedCount++
    Write-Host "T24 | PASS | Malformed CSV (Column count mismatch)" -ForegroundColor Green
} else {
    $failedCount++
    Write-Host "T24 | FAIL | Malformed CSV (Column count mismatch)" -ForegroundColor Red
}

# T25: Invalid Numeric Data in CSV
"id,name,cgpa,age`n1,Alice,NOT_A_FLOAT,20" | Out-File -Encoding ascii "students.csv"
$out25 = .\minisql.exe "SELECT name FROM students;" 2>&1 | Out-String
$exit25 = $LASTEXITCODE
if ($exit25 -eq 4 -and ($out25 -match "Data format error")) {
    $passedCount++
    Write-Host "T25 | PASS | Invalid numeric data in CSV" -ForegroundColor Green
} else {
    $failedCount++
    Write-Host "T25 | FAIL | Invalid numeric data in CSV" -ForegroundColor Red
}

# Restore original students.csv
Set-Content "students.csv" -Value $origCsvContent -NoNewline

# ======================================================================
# JSON Machine Interface Test (T26)
# ======================================================================
$jsonOut = .\minisql.exe --json "SELECT name, cgpa FROM students WHERE cgpa > 8.0;" 2>&1 | Out-String
$exit26 = $LASTEXITCODE
$validJson = $false
try {
    $parsed = ConvertFrom-Json $jsonOut
    if ($parsed.success -eq $true -and $parsed.stages.execution.rowCount -eq 3) {
        $validJson = $true
    }
} catch {
    $validJson = $false
}

if ($exit26 -eq 0 -and $validJson) {
    $passedCount++
    Write-Host "T26 | PASS | Real Compiler API JSON Bridge Interface" -ForegroundColor Green
} else {
    $failedCount++
    Write-Host "T26 | FAIL | Real Compiler API JSON Bridge Interface" -ForegroundColor Red
}

$totalTests = $tests.Count + 4
$summaryColor = if ($failedCount -eq 0) { "Green" } else { "Red" }
Write-Host "`n======================================================================" -ForegroundColor Cyan
Write-Host "TOTAL TESTS: $totalTests | PASSED: $passedCount | FAILED: $failedCount" -ForegroundColor $summaryColor
Write-Host "======================================================================`n" -ForegroundColor Cyan

if ($failedCount -gt 0) {
    exit 1
}
exit 0
