/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_HPP_INCLUDED
# define YY_YY_PARSER_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 25 "parser.y"

    #include <string>
    #include <vector>
    #include <utility>
    #include "ast.h"

#line 56 "parser.tab.hpp"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOKEN_SELECT = 258,            /* "'SELECT'"  */
    TOKEN_FROM = 259,              /* "'FROM'"  */
    TOKEN_WHERE = 260,             /* "'WHERE'"  */
    TOKEN_ORDER = 261,             /* "'ORDER'"  */
    TOKEN_BY = 262,                /* "'BY'"  */
    TOKEN_ASC = 263,               /* "'ASC'"  */
    TOKEN_DESC = 264,              /* "'DESC'"  */
    TOKEN_LIMIT = 265,             /* "'LIMIT'"  */
    TOKEN_AND = 266,               /* "'AND'"  */
    TOKEN_OR = 267,                /* "'OR'"  */
    TOKEN_COMMA = 268,             /* "','"  */
    TOKEN_SEMICOLON = 269,         /* "';'"  */
    TOKEN_LPAREN = 270,            /* "'('"  */
    TOKEN_RPAREN = 271,            /* "')'"  */
    TOKEN_EQ = 272,                /* "'='"  */
    TOKEN_NEQ = 273,               /* "'!='"  */
    TOKEN_LT = 274,                /* "'<'"  */
    TOKEN_GT = 275,                /* "'>'"  */
    TOKEN_LTE = 276,               /* "'<='"  */
    TOKEN_GTE = 277,               /* "'>='"  */
    TOKEN_IDENTIFIER = 278,        /* "identifier"  */
    TOKEN_INT = 279,               /* "integer literal"  */
    TOKEN_FLOAT = 280,             /* "number literal"  */
    TOKEN_STRING = 281,            /* "string literal"  */
    TOKEN_BOOL = 282,              /* "boolean literal"  */
    TOKEN_INVALID = 283            /* "invalid character"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 32 "parser.y"

    int int_val;
    char* str_val;
    ExprNode* expr_val;
    std::vector<std::string>* str_list_val;
    SelectQueryNode* query_val;
    std::pair<std::string, bool>* order_by_val;

#line 110 "parser.tab.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_HPP_INCLUDED  */
