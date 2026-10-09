CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra

# Fallback path if win_bison/win_flex are not in system PATH
WFB_DIR = $(LOCALAPPDATA)/Microsoft/WinGet/Packages/WinFlexBison.win_flex_bison_Microsoft.Winget.Source_8wekyb3d8bbwe

BISON ?= win_bison
FLEX ?= win_flex

ifeq ($(OS),Windows_NT)
    TARGET = minisql.exe
    RM = del /f /q
    NULL_DEV = 2>nul
else
    TARGET = minisql
    RM = rm -f
    NULL_DEV =
endif

all: $(TARGET)

parser.tab.cpp parser.tab.hpp: parser.y ast.h
	$(BISON) -d -o parser.tab.cpp parser.y

lex.yy.cpp: lexer.l parser.tab.hpp ast.h
	$(FLEX) -o lex.yy.cpp lexer.l

SRCS = main.cpp ast.cpp symbol_table.cpp semantic_analyzer.cpp ir.cpp optimizer.cpp executor.cpp parser.tab.cpp lex.yy.cpp
HEADERS = ast.h symbol_table.h semantic_analyzer.h ir.h optimizer.h executor.h parser.tab.hpp

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	$(RM) parser.tab.cpp parser.tab.hpp lex.yy.cpp $(TARGET) $(NULL_DEV)

.PHONY: all clean
