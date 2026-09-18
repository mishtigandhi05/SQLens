%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include "ast.h"

#ifndef strdup
  #define strdup my_strdup
#endif

// Lexer and parser declarations
int yylex();
void yyerror(const char* s);

extern int yylineno;
extern char* yytext;
extern bool g_lexical_error;
extern bool g_syntax_error;
%}

%define parse.error detailed

%code requires {
    #include <string>
    #include <vector>
    #include "ast.h"
}

%union {
    int int_val;
    char* str_val;
    ExprNode* expr_val;
    std::vector<std::string>* str_list_val;
    SelectQueryNode* query_val;
}

/* Keywords */
%token TOKEN_SELECT "'SELECT'"
%token TOKEN_FROM "'FROM'"
%token TOKEN_WHERE "'WHERE'"
%token TOKEN_ORDER "'ORDER'"
%token TOKEN_BY "'BY'"
%token TOKEN_LIMIT "'LIMIT'"
%token TOKEN_AND "'AND'"
%token TOKEN_OR "'OR'"

/* Punctuation */
%token TOKEN_COMMA "','"
%token TOKEN_SEMICOLON "';'"

/* Comparison Operators */
%token TOKEN_EQ "'='"
%token TOKEN_NEQ "'!='"
%token TOKEN_LT "'<'"
%token TOKEN_GT "'>'"
%token TOKEN_LTE "'<='"
%token TOKEN_GTE "'>='"

/* Literals & Identifiers */
%token <str_val> TOKEN_IDENTIFIER "identifier"
%token <str_val> TOKEN_INT "integer literal"
%token <str_val> TOKEN_FLOAT "number literal"
%token <str_val> TOKEN_STRING "string literal"
%token <str_val> TOKEN_BOOL "boolean literal"

/* Special token for lexical errors */
%token TOKEN_INVALID "invalid character"

/* Types for non-terminals */
%type <query_val> query
%type <str_list_val> column_list
%type <expr_val> opt_where condition expr
%type <str_val> opt_order_by comp_op
%type <int_val> opt_limit

/* Operator Precedence (lowest to highest) */
%left TOKEN_OR
%left TOKEN_AND
%left TOKEN_EQ TOKEN_NEQ TOKEN_LT TOKEN_GT TOKEN_LTE TOKEN_GTE

%start query

%%

query:
    TOKEN_SELECT column_list TOKEN_FROM TOKEN_IDENTIFIER opt_where opt_order_by opt_limit TOKEN_SEMICOLON {
        $$ = new SelectQueryNode(*$2, $4, $5, $6 ? $6 : "", $7);
        g_root_ast = $$;
        delete $2;
        free($4);
        if ($6) free($6);
    }
;

column_list:
    TOKEN_IDENTIFIER {
        $$ = new std::vector<std::string>();
        $$->push_back($1);
        free($1);
    }
  | column_list TOKEN_COMMA TOKEN_IDENTIFIER {
        $1->push_back($3);
        $$ = $1;
        free($3);
    }
;

opt_where:
    /* empty */ {
        $$ = nullptr;
    }
  | TOKEN_WHERE condition {
        $$ = $2;
    }
;

opt_order_by:
    /* empty */ {
        $$ = nullptr;
    }
  | TOKEN_ORDER TOKEN_BY TOKEN_IDENTIFIER {
        $$ = $3;
    }
;

opt_limit:
    /* empty */ {
        $$ = -1;
    }
  | TOKEN_LIMIT TOKEN_INT {
        $$ = std::stoi($2);
        free($2);
    }
;

condition:
    condition TOKEN_AND condition {
        $$ = new BinaryOpNode("AND", $1, $3);
    }
  | condition TOKEN_OR condition {
        $$ = new BinaryOpNode("OR", $1, $3);
    }
  | expr comp_op expr {
        $$ = new BinaryOpNode($2, $1, $3);
        free($2);
    }
;

comp_op:
    TOKEN_EQ  { $$ = strdup("="); }
  | TOKEN_NEQ { $$ = strdup("!="); }
  | TOKEN_LT  { $$ = strdup("<"); }
  | TOKEN_GT  { $$ = strdup(">"); }
  | TOKEN_LTE { $$ = strdup("<="); }
  | TOKEN_GTE { $$ = strdup(">="); }
;

expr:
    TOKEN_IDENTIFIER {
        $$ = new ColumnRefNode($1);
        free($1);
    }
  | TOKEN_INT {
        $$ = new LiteralNode($1, "INT");
        free($1);
    }
  | TOKEN_FLOAT {
        $$ = new LiteralNode($1, "FLOAT");
        free($1);
    }
  | TOKEN_STRING {
        $$ = new LiteralNode($1, "STRING");
        free($1);
    }
  | TOKEN_BOOL {
        $$ = new LiteralNode($1, "BOOLEAN");
        free($1);
    }
;

%%

bool g_syntax_error = false;
std::string g_syntax_error_msg = "";

void yyerror(const char* s) {
    // Suppress secondary syntax error if a lexical error already occurred
    if (g_lexical_error) {
        return;
    }
    g_syntax_error = true;
    std::string msg = s ? s : "syntax error";
    if (msg.rfind("syntax error, ", 0) == 0) {
        msg = msg.substr(14); // Remove redundant prefix
    }
    g_syntax_error_msg = "At line " + std::to_string(yylineno) + ": " + msg;
}
