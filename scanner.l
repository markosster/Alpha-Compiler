%{
/* -------- Parameters, Definitions -------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"

/*#define YY_DECL int alpha_yylex (void* yylval)*/
extern unsigned int count_digits(unsigned int);

typedef struct alpha_token_t {
  unsigned int          numLine;
  unsigned int          numToken;
  char                  *content;
  char                  *type[3];
  struct alpha_token_t  *next;
} token;

token *token_list = NULL;
token *last_token = NULL;

/* token counter */
int count_token = 0;

/* comment lines counter */
int start_multicom_line;
int end_multicom_line;
int start_nestcom_line;
int nested_blocks = 0;

/* string lines counter */
int start_str_line = 0;

/* string buffer */
char *buf;
unsigned int size = 0;

/* error flag */
int found_error = 0;

/*
    inserts in the list of tokens
*/
int add_token (int numLine, int numToken, char *content, char *type1, char *type2, char *type3) {
    
    token *newToken = (token *) malloc(sizeof(token));

    assert(newToken != NULL);

    newToken->numLine = numLine;
    newToken->numToken = numToken;
    newToken->content = strdup(content);
    newToken->type[0] = strdup(type1);
    newToken->type[1] = strdup(type2);
    newToken->type[2] = strdup(type3);
    newToken->next = NULL;

	if (token_list == NULL) {
		token_list = newToken;
        last_token = newToken;
    } else {
        last_token->next = newToken;
		last_token = newToken;
    }

    return 1;
}

/*
    prints the token's list
*/
void print_list () {
    token *tok = token_list;

    fprintf(yyout,   "+----------------------------------------------------------+\n");
    fprintf(yyout,   "|------------------   Lexical Analysis   ------------------|\n");
    fprintf(yyout,   "+----------------------------------------------------------+\n\n");

	while(tok != NULL)
	{
        if (!strcmp(tok->type[0],"ID") || !strcmp(tok->type[0],"STRING"))
            fprintf(yyout,"%d: #%d \"%s\" %s \"%s\" <- %s\n", tok->numLine, tok->numToken, tok->content, tok->type[0], tok->type[1], tok->type[2]);
		else
            fprintf(yyout,"%d: #%d \"%s\" %s %s <- %s\n", tok->numLine, tok->numToken, tok->content, tok->type[0], tok->type[1], tok->type[2]);
		tok = tok->next;
	}
}

/*
    free memory
*/
int free_list () {
    token *tok = token_list;

    while (tok != NULL) {
        token *nextToken = tok->next;
        free(tok->content);
        free(tok->type[0]);
        free(tok->type[1]);
        free(tok->type[2]);
		free(tok);
		tok = nextToken;
    }

    token_list = NULL;
    last_token = NULL;

    return 1;
}

/*
    error function
*/
void lex_error (char *message, int lineno) {
    found_error = 1;
    fprintf(stderr, "\033[0;31m");
    fprintf(stderr, "LEXICAL ERROR: %s in line %d. Token = \"%s\"\n", message, lineno, yytext);
    fprintf(stderr, "\033[0m");
}

/*
    makes a string with the start and the end line number of a multiline comment
*/
char* comment_lines_str (int start_multicom_line, int end_multicom_line) {
    int start_digits = count_digits(start_multicom_line);
    int end_digits = count_digits(end_multicom_line);
    char *str_lines = malloc(start_digits + end_digits + 10);
    memset(str_lines, 0, sizeof(str_lines));

    if (start_multicom_line == end_multicom_line)
        sprintf(str_lines, "line %d", start_multicom_line);
    else
        sprintf(str_lines, "lines %d - %d", start_multicom_line, end_multicom_line);

    return str_lines;
}

%}

%option noyywrap
%option yylineno

/* --- Regular Expressions --- */

/* constants */
DIGIT       [0-9]
INTCONST    {DIGIT}+
REALCONST   {DIGIT}+"."{DIGIT}*

/* id */
ID          [a-zA-Z][a-zA-Z0-9_]*

/* whitespaces */
WHITESPACES  [ \r\n\t\v\f\b]+
UNDEFINED   .

/* line comments */
LINE_COMMENT    "//".*

/* multiple line comments exclusive condition state */
%x MULTI_COMMENT

/* string exclusive condition state */
%x STRING_CHECK

/* ------ Rules block ------ */
%%

"if"            { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",       "IF", "enumerated");*/ return IF;       }
"else"          { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",     "ELSE", "enumerated");*/ return ELSE;     }
"while"         { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",    "WHILE", "enumerated");*/ return WHILE;    }
"for"           { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",      "FOR", "enumerated");*/ return FOR;      }
"function"      { /*add_token(yylineno, ++count_token, yytext, "KEYWORD", "FUNCTION", "enumerated");*/ return FUNCTION; }
"return"        { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",   "RETURN", "enumerated");*/ return RETURN;   }
"break"         { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",    "BREAK", "enumerated");*/ return BREAK;    }
"continue"      { /*add_token(yylineno, ++count_token, yytext, "KEYWORD", "CONTINUE", "enumerated");*/ return CONTINUE; }
"and"           { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",      "AND", "enumerated");*/ return AND;      }
"not"           { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",      "NOT", "enumerated");*/ return NOT;      }
"or"            { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",       "OR", "enumerated");*/ return OR;       }
"local"         { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",    "LOCAL", "enumerated");*/ return local;    }
"true"          { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",     "TRUE", "enumerated");*/ return TRUE;     }
"false"         { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",    "FALSE", "enumerated");*/ return FALSE;    }
"nil"           { /*add_token(yylineno, ++count_token, yytext, "KEYWORD",      "NIL", "enumerated");*/ return NIL;      }

"="             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR", "ASSIGN", "enumerated");*/ return ASSIGN;   }
"+"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",   "PLUS", "enumerated");*/ return PLUS;     }
"-"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",  "MINUS", "enumerated");*/ return MINUS;    }
"*"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",    "MUL", "enumerated");*/ return MUL;      }
"/"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",    "DIV", "enumerated");*/ return DIV;      }
"%"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",    "MOD", "enumerated");*/ return MOD;      }
"=="            { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",  "EQUAL", "enumerated");*/ return EQUAL;    }
"!="            { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",  "NOTEQ", "enumerated");*/ return NOTEQUAL; }
"++"            { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",   "INCR", "enumerated");*/ return INCR;     }
"--"            { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",   "DECR", "enumerated");*/ return DECR;     }
">"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",     "GT", "enumerated");*/ return GT;       }
"<"             { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",     "LT", "enumerated");*/ return LT;       }
">="            { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",     "GE", "enumerated");*/ return GE;       }
"<="            { /*add_token(yylineno, ++count_token, yytext, "OPERATOR",     "LE", "enumerated");*/ return LE;       }

"{"             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",      "LBRACE", "enumerated");*/ return LBRACE;       }
"}"             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",      "RBRACE", "enumerated");*/ return RBRACE;       }
"["             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",    "LBRACKET", "enumerated");*/ return LBRACKET;     }
"]"             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",    "RBRACKET", "enumerated");*/ return RBRACKET;     }
"("             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",      "LPAREN", "enumerated");*/ return LPAREN;       }
")"             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",      "RPAREN", "enumerated");*/ return RPAREN;       }
";"             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",   "SEMICOLON", "enumerated");*/ return SEMICOLON;    }
","             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",       "COMMA", "enumerated");*/ return COMMA;        }
":"             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",       "COLON", "enumerated");*/ return COLON;        }
"::"            { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION","DOUBLE_COLON", "enumerated");*/ return DOUBLE_COLON; }
"."             { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",         "DOT", "enumerated");*/ return DOT;          }
".."            { /*add_token(yylineno, ++count_token, yytext, "PUNCTUATION",  "DOUBLE_DOT", "enumerated");*/ return DOUBLE_DOT;   }

{ID}            { /*add_token(yylineno, ++count_token, yytext,        "ID", yytext, "char*");*/ yylval.strVal = strdup(yytext); return ID;          }
{INTCONST}      { /*add_token(yylineno, ++count_token, yytext,  "INTCONST", yytext,   "int");*/ yylval.intNum = atoi(yytext);   return INTCONST;    }
{REALCONST}     { /*add_token(yylineno, ++count_token, yytext, "REALCONST", yytext, "float");*/ yylval.realNum = atof(yytext);  return REALCONST;   }


\"              {
    BEGIN(STRING_CHECK);
    buf = malloc(sizeof(char));
    size = 1;
    start_str_line = yylineno;
}

<STRING_CHECK>{

    \"          {
        BEGIN(INITIAL);
        buf[size-1] = '\0';
        yylval.strVal = strdup(buf);
        /*add_token(start_str_line, ++count_token, buf, "STRING", buf, "char*");*/
        free(buf);
        buf = NULL;
        return STRING;
    }

    /* escape characters */
    \\n|\n      { buf = realloc(buf, ++size); buf[size-2] = '\n';  }
    \\r|\r      { buf = realloc(buf, ++size); buf[size-2] = '\r';  }
    \\t|\t      { buf = realloc(buf, ++size); buf[size-2] = '\t';  }
    \\["]       { buf = realloc(buf, ++size); buf[size-2] = '\"';  }
    \\[\\]      { buf = realloc(buf, ++size); buf[size-2] = '\\';  }
    \\.         { lex_error("Unknown escape character", yylineno); }

    /* save everything between escape characters */
    [^\\\n\t\"]+  {
        int new_str_sz = strlen(yytext);
        size = size + new_str_sz;
        buf = realloc(buf, size);
        memset(buf+(size-new_str_sz-1), 0, new_str_sz);
        strcat(buf, yytext);
    }

    /* we found EOF but the current string didnt close properly */
    <<EOF>>     { 
        lex_error("Unterminated String", start_str_line);
        free(buf);
        buf = NULL;
        BEGIN(INITIAL);
    }
}

{WHITESPACES}       { } /* do nothing, skip all the whitespace characters */
{UNDEFINED}         { lex_error("Undefined character found", yylineno); }

{LINE_COMMENT}      { /*add_token(yylineno, ++count_token, "", "COMMENT", "LINE_COMMENT", "enumerated");*/ }

"/*"                        {
    BEGIN(MULTI_COMMENT);
    start_multicom_line = yylineno;
    end_multicom_line = yylineno;
}

<MULTI_COMMENT>"/*"         { ++nested_blocks; start_nestcom_line = yylineno; } /* count opened nested blocks */
<MULTI_COMMENT>"*"+         { } /* eat anything that is '*' (possible asterisk sequences) */
<MULTI_COMMENT>[^/*\n]+     { } /* eat anything between open comment character (/ *) and newline (\n) */
<MULTI_COMMENT>[/]          { } /* eat '/' characters that is inside a comment block */
<MULTI_COMMENT>\n           { ++end_multicom_line; } /* count newlines */
<MULTI_COMMENT>"*/"         {

    /*char* str;*/

    if (nested_blocks) {
        --nested_blocks;
        /*str = comment_lines_str(start_nestcom_line, end_multicom_line);
        add_token(start_multicom_line, ++count_token, str, "COMMENT", "NESTED_BLOCK_COMMENT", "enumerated");*/
    } else {
        BEGIN(INITIAL);
        /*str = comment_lines_str(start_multicom_line, end_multicom_line);
        add_token(start_multicom_line, ++count_token, str, "COMMENT", "BLOCK_COMMENT", "enumerated");*/
    }

    /*free(str);*/
}

<MULTI_COMMENT><<EOF>>      { lex_error("Unterminated Multiline Comment", start_multicom_line); BEGIN(INITIAL); }

%%