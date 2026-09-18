%{
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "ast.h"
#include "parser.tab.hpp"

#ifndef strdup
  #define strdup my_strdup
#endif

bool g_print_tokens = false;
bool g_lexical_error = false;
std::string g_lexical_error_msg = "";
std::vector<std::string> g_recorded_tokens;

// Helper to record token stream and return token code to Bison
static int emit(int token_id, const char* token_label) {
    std::string text(yytext ? yytext : "");
    std::string repr = "";

    if (strcmp(token_label, "KEYWORD") == 0) {
        std::string upper = text;
        for (char &c : upper) c = (char)std::toupper((unsigned char)c);
        repr = upper;
    } else if (strcmp(token_label, "COMMA") == 0) {
        repr = "COMMA";
    } else if (strcmp(token_label, "SEMICOLON") == 0) {
        repr = "SEMICOLON";
    } else if (strcmp(token_label, "EQUALS") == 0) {
        repr = "EQUALS";
    } else if (strcmp(token_label, "NOT_EQUALS") == 0) {
        repr = "NOT_EQUALS";
    } else if (strcmp(token_label, "GREATER_THAN") == 0) {
        repr = "GREATER";
    } else if (strcmp(token_label, "LESS_THAN") == 0) {
        repr = "LESS";
    } else if (strcmp(token_label, "GREATER_EQUAL") == 0) {
        repr = "GREATER_EQUAL";
    } else if (strcmp(token_label, "LESS_EQUAL") == 0) {
        repr = "LESS_EQUAL";
    } else if (strcmp(token_label, "IDENTIFIER") == 0) {
        repr = "IDENTIFIER(" + text + ")";
    } else if (strcmp(token_label, "INTEGER_LITERAL") == 0) {
        repr = "INTEGER_LITERAL(" + text + ")";
    } else if (strcmp(token_label, "FLOAT_LITERAL") == 0) {
        repr = "FLOAT_LITERAL(" + text + ")";
    } else if (strcmp(token_label, "STRING_LITERAL") == 0) {
        repr = "STRING_LITERAL(" + text + ")";
    } else if (strcmp(token_label, "BOOLEAN_LITERAL") == 0) {
        repr = "BOOLEAN_LITERAL(" + text + ")";
    } else {
        repr = token_label;
    }

    g_recorded_tokens.push_back(repr);
    if (g_print_tokens) {
        std::cout << repr << "\n";
    }
    return token_id;
}
%}

%option noyywrap
%option yylineno
%option case-insensitive
%option nounistd
%option never-interactive
%option nounput

/* Regular Expression Definitions */
WHITESPACE       [ \t\r\n]+
COMMENT          "--"[^\r\n]*
IDENTIFIER       [a-zA-Z_][a-zA-Z0-9_]*
INTEGER_LITERAL  [0-9]+
FLOAT_LITERAL    [0-9]+\.[0-9]+
STRING_LITERAL   \'([^'\\]|\\.)*\'|\"([^"\\]|\\.)*\"

%%

{WHITESPACE}     { /* Ignore whitespace */ }
{COMMENT}        { /* Ignore single-line comments */ }

"SELECT"         { return emit(TOKEN_SELECT, "KEYWORD"); }
"FROM"           { return emit(TOKEN_FROM, "KEYWORD"); }
"WHERE"          { return emit(TOKEN_WHERE, "KEYWORD"); }
"ORDER"          { return emit(TOKEN_ORDER, "KEYWORD"); }
"BY"             { return emit(TOKEN_BY, "KEYWORD"); }
"LIMIT"          { return emit(TOKEN_LIMIT, "KEYWORD"); }
"AND"            { return emit(TOKEN_AND, "KEYWORD"); }
"OR"             { return emit(TOKEN_OR, "KEYWORD"); }

"TRUE"           { 
                   yylval.str_val = strdup("TRUE");
                   return emit(TOKEN_BOOL, "BOOLEAN_LITERAL"); 
                 }
"FALSE"          { 
                   yylval.str_val = strdup("FALSE");
                   return emit(TOKEN_BOOL, "BOOLEAN_LITERAL"); 
                 }

","              { return emit(TOKEN_COMMA, "COMMA"); }
";"              { return emit(TOKEN_SEMICOLON, "SEMICOLON"); }

"="              { return emit(TOKEN_EQ, "EQUALS"); }
"!="             { return emit(TOKEN_NEQ, "NOT_EQUALS"); }
">="             { return emit(TOKEN_GTE, "GREATER_EQUAL"); }
"<="             { return emit(TOKEN_LTE, "LESS_EQUAL"); }
">"              { return emit(TOKEN_GT, "GREATER_THAN"); }
"<"              { return emit(TOKEN_LT, "LESS_THAN"); }

{FLOAT_LITERAL}  {
                   yylval.str_val = strdup(yytext);
                   return emit(TOKEN_FLOAT, "FLOAT_LITERAL");
                 }

{INTEGER_LITERAL} {
                   yylval.str_val = strdup(yytext);
                   return emit(TOKEN_INT, "INTEGER_LITERAL");
                 }

{STRING_LITERAL} {
                   // Strip outer quotation marks
                   std::string raw(yytext);
                   std::string stripped = raw.substr(1, raw.size() - 2);
                   yylval.str_val = strdup(stripped.c_str());
                   return emit(TOKEN_STRING, "STRING_LITERAL");
                 }

{IDENTIFIER}     {
                   yylval.str_val = strdup(yytext);
                   return emit(TOKEN_IDENTIFIER, "IDENTIFIER");
                 }

.                {
                   g_lexical_error = true;
                   if (g_lexical_error_msg.empty()) {
                       g_lexical_error_msg = "Invalid character '" + std::string(yytext) + "' at line " + std::to_string(yylineno) + ".";
                   }
                   return TOKEN_INVALID;
                 }

%%

static YY_BUFFER_STATE g_current_buffer = NULL;

void set_lexer_input_string(const char* str) {
    g_current_buffer = yy_scan_string(str);
}

void clean_lexer_input() {
    if (g_current_buffer) {
        yy_delete_buffer(g_current_buffer);
        g_current_buffer = NULL;
    }
}

void clear_lexer_state() {
    g_lexical_error = false;
    g_lexical_error_msg.clear();
    g_recorded_tokens.clear();
}
