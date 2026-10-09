# MiniSQL Prototype Build Script for Windows PowerShell

$ErrorActionPreference = "Stop"

# Detect win_bison / bison
$bisonCmd = Get-Command "win_bison" -ErrorAction SilentlyContinue
if (-not $bisonCmd) {
    $bisonCmd = Get-Command "bison" -ErrorAction SilentlyContinue
}
if (-not $bisonCmd) {
    $wingetDir = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\WinFlexBison.win_flex_bison_Microsoft.Winget.Source_8wekyb3d8bbwe"
    if (Test-Path "$wingetDir\win_bison.exe") {
        $bisonPath = "$wingetDir\win_bison.exe"
    } else {
        Write-Error "Neither win_bison nor bison was found."
    }
} else {
    $bisonPath = $bisonCmd.Source
}

# Detect win_flex / flex
$flexCmd = Get-Command "win_flex" -ErrorAction SilentlyContinue
if (-not $flexCmd) {
    $flexCmd = Get-Command "flex" -ErrorAction SilentlyContinue
}
if (-not $flexCmd) {
    $wingetDir = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\WinFlexBison.win_flex_bison_Microsoft.Winget.Source_8wekyb3d8bbwe"
    if (Test-Path "$wingetDir\win_flex.exe") {
        $flexPath = "$wingetDir\win_flex.exe"
    } else {
        Write-Error "Neither win_flex nor flex was found."
    }
} else {
    $flexPath = $flexCmd.Source
}

# Detect g++
$gppCmd = Get-Command "g++" -ErrorAction SilentlyContinue
if (-not $gppCmd) {
    if (Test-Path "C:\MinGW\bin\g++.exe") {
        $gppPath = "C:\MinGW\bin\g++.exe"
    } else {
        Write-Error "g++ compiler was not found."
    }
} else {
    $gppPath = $gppCmd.Source
}

Write-Host "=== Building MiniSQL Compiler ===" -ForegroundColor Cyan
Write-Host "Using Bison : $bisonPath"
Write-Host "Using Flex  : $flexPath"
Write-Host "Using G++   : $gppPath"

Write-Host "`n[1/3] Generating parser from parser.y..." -ForegroundColor Yellow
& $bisonPath -d -o parser.tab.cpp parser.y
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[2/3] Generating lexer from lexer.l..." -ForegroundColor Yellow
& $flexPath -o lex.yy.cpp lexer.l
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[3/3] Compiling C++ sources..." -ForegroundColor Yellow
& $gppPath -std=c++17 -Wall -Wextra -o minisql.exe main.cpp ast.cpp symbol_table.cpp semantic_analyzer.cpp ir.cpp optimizer.cpp executor.cpp parser.tab.cpp lex.yy.cpp
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`nBuild successful! Generated minisql.exe" -ForegroundColor Green
