%{
/* -- Prologue -- */
#include "manager.c"

#define YY_DECL int alpha_yylex (void* yylval)
int yylex(void);

extern int yylex_destroy(void);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;
extern FILE* yyout;

void yyerror (char* yaccProvidedMessage) {
        COMPILE_ERROR(stderr, "%s: in line %d before token \"%s\"", yaccProvidedMessage, yylineno, yytext)
}

%}
/* -- Definitions -- */
%start program

/* Keyword Tokens */
%token IF
%token ELSE
%token WHILE
%token FOR
%token FUNCTION
%token RETURN
%token BREAK
%token CONTINUE
%token AND
%token NOT
%token OR
%token local
%token TRUE
%token FALSE
%token NIL

/* Operator Tokens */
%token ASSIGN
%token PLUS
%token MINUS
%token MUL
%token DIV
%token MOD
%token EQUAL
%token NOTEQUAL
%token INCR
%token DECR
%token GT
%token LT
%token GE
%token LE

/* Punctuation Tokens */
%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET
%token LPAREN
%token RPAREN
%token SEMICOLON
%token COMMA
%token COLON
%token DOUBLE_COLON
%token DOT
%token DOUBLE_DOT

%code requires { 
        #include "symtable/symtable.h"
        #include "icode/expr.h"
        #include "icode/quadlists.h"
}

%union { 
        unsigned int uinteger; 
        int intNum;
        double realNum;
        char* strVal;
        SymTableEntry *symbol;
        Expr *expr;
        F_call fcall;
        HashExprObject *hashObj;
        stmt statement;
        forprefix_s forprefix_type;
}

/* Constants */
%token <realNum> REALCONST
%token <intNum>  INTCONST
%token <strVal>  ID
%token <strVal>  STRING

/* Types */
%type <symbol> funcdef
%type <symbol> funcprefix
%type <symbol> funcname
/* ---------------- */
%type <expr> lvalue
%type <expr> member
%type <expr> primary
%type <expr> assignexpr
%type <expr> call
%type <expr> term
%type <expr> tabledef
%type <expr> const
%type <expr> elist
%type <expr> expr
%type <expr> tableitem
/* ---------------- */
%type <fcall> callsuffix
%type <fcall> normcall
%type <fcall> methodcall
/* ---------------- */
%type <hashObj> indexedelem
%type <hashObj> indexed
/* ---------------- */
%type <uinteger> M N S
%type <uinteger> ifprefix
%type <uinteger> elseprefix
%type <uinteger> whilestart
%type <uinteger> whilecond
%type <uinteger> funcbody
%type <uinteger> funcargs
/* ---------------- */
%type <statement> error
%type <statement> stmts
%type <statement> stmt
%type <statement> loopbody
%type <statement> whilestmt
%type <statement> ifstmt
%type <statement> forstmt
%type <statement> returnstmt
%type <statement> block
%type <statement> break
%type <statement> continue
/* ---------------- */
%type <forprefix_type> forprefix

/* Priority Rules */
%right ASSIGN                /* = */
%left OR                     /* or */
%left AND                    /* and */
%nonassoc EQUAL NOTEQUAL     /* == != */
%nonassoc GT GE LT LE        /* > >= < <= */
%left PLUS MINUS             /* + - */
%left MUL DIV MOD            /* * / % */
%right NOT INCR DECR UMINUS  /* not ++ -- - */
%left DOT DOUBLE_DOT         /* . .. */
%left LBRACKET RBRACKET      /* [ ] */
%left LPAREN RPAREN          /* ( ) */

/* Expected conflicts: 1 (for if statement) */
%expect 1

%%

program:        stmts
                ;
        
stmts:          stmt { Quads_resetTemp(); } stmts { $$ = manage_stmts($$, $1, $3); }
                | error stmts           /* in case of grammar error, it consumes every token until it founds an empty "stmts" */
                | /* empty */           { Stmt_makeList(&$stmts); }
                ;

stmt:           expr SEMICOLON          { $expr = emit_ifboolexpr($expr); Stmt_makeList(&$stmt); }
                | ifstmt                { $stmt = $ifstmt; }
                | whilestmt             { $stmt = $whilestmt; }
                | forstmt               { $stmt = $forstmt; }
                | returnstmt            { $stmt = $returnstmt; }
                | break                 { $stmt = $break; }
                | continue              { $stmt = $continue; }
                | block                 { $stmt = $block; }
                | funcdef               { Stmt_makeList(&$stmt); }
                | SEMICOLON             { Stmt_makeList(&$stmt); }
                ;

break:          BREAK SEMICOLON         { $break = manage_break($break); } ;
continue:       CONTINUE SEMICOLON      { $continue = manage_continue($continue); } ;

expr:           assignexpr              { $expr = $assignexpr; }
                | expr PLUS expr        { $$ = manage_expr_arithop_expr($1, add, $3); }
                | expr MINUS expr       { $$ = manage_expr_arithop_expr($1, sub, $3); }
                | expr MUL expr         { $$ = manage_expr_arithop_expr($1, mul, $3); }
                | expr DIV expr         { $$ = manage_expr_arithop_expr($1, div_op, $3); }
                | expr MOD expr         { $$ = manage_expr_arithop_expr($1, mod, $3); }
                | expr GT expr          { $$ = manage_expr_relop_expr($1, if_greater, $3); }
                | expr GE expr          { $$ = manage_expr_relop_expr($1, if_greatereq, $3); }
                | expr LT expr          { $$ = manage_expr_relop_expr($1, if_less, $3); }
                | expr LE expr          { $$ = manage_expr_relop_expr($1, if_lesseq, $3); }
                | expr EQUAL expr       { $$ = manage_expr_relop_expr($1, if_eq, $3); }
                | expr NOTEQUAL expr    { $$ = manage_expr_relop_expr($1, if_noteq, $3); }
                | expr AND { if ($1) true_test(&$1); } S expr { $$ = manage_expr_boolop_expr($1, and, $S, $5); }
                | expr OR  { if ($1) true_test(&$1); } S expr { $$ = manage_expr_boolop_expr($1,  or, $S, $5); }
                | NOT expr              { $$ = manage_expr_boolop_expr(NULL, not, 0, $2); }
                | term                  { $expr = $term; }
                ;

term:           LPAREN expr RPAREN      { $term = $expr; }
                | MINUS expr            { $term = manage_term_prefix($expr, uminus); } %prec UMINUS
                | INCR lvalue           { $term = manage_term_prefix($lvalue, add); }
                | lvalue INCR           { $term = manage_term_postfix($lvalue, add); }
                | DECR lvalue           { $term = manage_term_prefix($lvalue, sub); }
                | lvalue DECR           { $term = manage_term_postfix($lvalue, sub); }
                | primary               { $term = $primary; }
                ;

assignexpr:     lvalue                  { check_funcid($lvalue); }
                ASSIGN expr             { $expr = emit_ifboolexpr($expr); $assignexpr = manage_assignexpr($lvalue, $expr); }
                ;

primary:        lvalue                  { $primary = manage_primary_lvalue($lvalue); }
                | call                  { $primary = $call; }
                | tabledef              { $primary = $tabledef; }
                | LPAREN funcdef RPAREN { $primary = manage_primary_funcdef($funcdef); }
                | const                 { $primary = $const; }
                ;

lvalue:         ID                      { $lvalue = manage_lvalue_id($ID); }
                | local ID              { $lvalue = manage_lvalue_local_id($ID); }
                | DOUBLE_COLON ID       { $lvalue = manage_lvalue_global_id($ID); }
                | member                { $lvalue = $member; }
                | tableitem             { $lvalue = $tableitem; }
                ;

tableitem:      lvalue DOT ID                   { $tableitem = manage_table_lvalue_dot_id($lvalue, $ID); }
                | lvalue LBRACKET expr RBRACKET { $tableitem = manage_table_lvalue_index_expr($lvalue, $expr); }
                ;

tabledef:       LBRACKET elist RBRACKET         { $tabledef = manage_objectdef_elist($elist); }
                | LBRACKET { insideIndexed = 1; } indexed { insideIndexed = 0; } RBRACKET { $tabledef = manage_objectdef_indexed($indexed); }
                ;

member:         call DOT ID                     { $member = manage_table_lvalue_dot_id($call, $ID); }
                | call LBRACKET expr RBRACKET   { $member = manage_table_lvalue_index_expr($call, $expr); }
                ;

call:           call LPAREN elist RPAREN        { $$ = manage_call_call_elist($1, $elist); }
                | lvalue callsuffix             { $$ = manage_call_lvalue_callsuffix($lvalue, $callsuffix); }
                | LPAREN funcdef RPAREN LPAREN elist RPAREN { $$ = manage_call_funcdef_elist($funcdef, $elist); }
                ;

callsuffix:     normcall                { $callsuffix = $normcall; }
                | methodcall            { $callsuffix = $methodcall; }
                ;

normcall:       LPAREN elist RPAREN     { $normcall = manage_normcall($normcall, $elist); }
                ;

methodcall:     DOUBLE_DOT ID LPAREN elist RPAREN { $methodcall = manage_methodcall($methodcall, $ID, $elist); }
                ;

elist:          elist COMMA expr        { $expr = emit_ifboolexpr($expr); $$ = manage_elist_comma_expr($1, $expr); }
                | expr                  { $expr = emit_ifboolexpr($expr); $elist = $expr; }
                | /* empty */           { $elist = (Expr *) NULL; }
                ;

indexed:        indexedelem                     { manage_indexed($$, $1, NULL); }
                | indexedelem COMMA indexed     { manage_indexed($$, $1, $3); }
                ;

indexedelem:    LBRACE expr COLON expr RBRACE   { $indexedelem = HashExprObject_new($2, $4); }
                ;

block:          LBRACE { scope++; } stmts RBRACE { ScopeList_hide(symtable->scopeList, scope--); $block = $stmts; }
                ;

funcdef:        funcprefix              { $funcprefix = manage_funcdef_funcprefix($funcprefix); }
                funcargs funcbody       { $funcdef = manage_funcdef_funcbody($funcprefix, $funcargs, $funcbody); }
                ;

funcprefix:     FUNCTION funcname { $funcprefix = $funcname; }
                ;

funcname:       ID              { $funcname = manage_funcname_id($ID); }
                | /* empty */   { $funcname = manage_funcname_anonymous(); }
                ;

funcargs:       LPAREN idlist RPAREN { $funcargs = currScopeOffset(); enterScopeSpace(); resetFunctionLocalsOffset(); }
                ;

funcbody:       { manage_funcbodystart(); } block { $funcbody = manage_funcbodyend(); }
                ;

const:          REALCONST       { $const = Expr_numConst($REALCONST); }
                | INTCONST      { $const = Expr_numConst($INTCONST); }
                | STRING        { $const = Expr_strConst(&$STRING); }
                | NIL           { $const = Expr_new(nil_e, NULL); }
                | TRUE          { $const = Expr_boolConst(1); }
                | FALSE         { $const = Expr_boolConst(0); }
                ;

idlist:         /* empty */
                | ID { manage_idlist_id($ID); } COMMA idlist
                | ID { manage_idlist_id($ID); }
                ;

ifstmt:         ifprefix stmt { $ifstmt = manage_ifprefix_stmt($ifprefix, $stmt); }
                | ifprefix stmt elseprefix stmt { $ifstmt = manage_ifelseprefix_stmt($$, $ifprefix, $2, $elseprefix, $4); }
                ;

ifprefix:       IF LPAREN expr RPAREN { $expr = emit_ifboolexpr($expr); $ifprefix = manage_ifprefix($expr); }
                ;

elseprefix:     ELSE { $elseprefix = manage_elseprefix(); }
                ;

whilestart:     WHILE { $whilestart = manage_whilestart(); }
                ;

whilecond:      LPAREN expr RPAREN { $expr = emit_ifboolexpr($expr); $whilecond = manage_whilecond($expr); }
                ;

whilestmt:      whilestart whilecond loopbody { $whilestmt = manage_whilestmt($whilestart, $whilecond, &$loopbody); }
                ;

S:              /* empty */ { $S = manage_S(); } ;
M:              /* empty */ { $M = manage_M(); } ;
N:              /* empty */ { $N = manage_N(); } ;

forprefix:      FOR LPAREN elist SEMICOLON M expr SEMICOLON { $expr = emit_ifboolexpr($expr); $forprefix = manage_forprefix($forprefix, $M, $expr); }
                ;

forstmt:        forprefix N elist RPAREN N loopbody N { $forstmt = manage_forstmt($forprefix, $2, $5, &$loopbody, $7); }
                ;

loopbodystart:  /* empty */ { ++loopCounter; } ;
loopbodyend:    /* empty */ { --loopCounter; } ;

loopbody:       loopbodystart stmt loopbodyend { $loopbody = $stmt; }
                ;

returnstmt:     RETURN expr SEMICOLON   { $expr = emit_ifboolexpr($expr); $returnstmt = manage_returnstmt($returnstmt, $expr); }
                | RETURN SEMICOLON      { $returnstmt = manage_returnstmt($returnstmt, NULL); }
                ;

%%

int main (int argc , char** argv) {

        FILE *quads_file = NULL, *binary_file = NULL, *binary_text_file = NULL;

        if (argc == 2) {
                if (!(yyin = fopen(argv[1], "r"))) {
                        PRINT_RED(stderr, "Cannot open file to read: %s\n", argv[1])
                        return 1;
                }
        } else {
                PRINT_RED(stderr, "Wrong arguments. Give input file.\n")
                return 1;
        }

        symtable = SymTable_init();

        /*PRINT_CYAN(stderr, "\n=> PARSING <=\n");*/
        yyparse();

        if (icode_status)
        {
                /*PRINT_CYAN(stderr, "\n=> SCOPES <=\n")
                ScopeList_display(stderr, symtable);*/
                
                /*PRINT_CYAN(stderr, "\n=> SYMTABLE <=\n")
                SymTable_display(stderr, symtable);*/

                /*PRINT_CYAN(stderr, "\n=> QUADS <=\n")
                Quads_display(stderr, 1);
                if (!(quads_file = fopen("quads.txt", "w"))){
                        PRINT_RED(stderr, "Cannot open file to write.\n")
                        return 1;
                }
                write_quads(quads_file);*/
                /*PRINT_YELLOW(stderr, "Quads file (quads.txt) created.\n")*/

                generate_tcode();
                /*PRINT_CYAN(stderr, "\n=> TCODE <=\n");*/
                if (!(binary_file = fopen("binary.abc", "w"))){
                        PRINT_RED(stderr, "Cannot open file to write.\n")
                        return 1;
                }
                write_binary(binary_file);
                /*PRINT_YELLOW(stderr, "Binary file (binary.abc) created.\n")*/

                /*if (!(binary_text_file = fopen("binary.abc.txt", "w"))){
                        PRINT_RED(stderr, "Cannot open file to write.\n")
                        return 1;
                }
                write_binary_text(binary_text_file);*/
                /*PRINT_YELLOW(stderr, "Text file (tcode.txt) created.\n")*/
        }
        /*print_icode_status();
        print_tcode_status();*/
        
        Instr_free();
        Consts_free();
        Quads_free();
        SymTable_free(symtable);

        fclose(yyin);
        if (quads_file) fclose(quads_file);
        if (binary_file) fclose(binary_file);
        if (binary_text_file) fclose(binary_text_file);

        yylex_destroy();

        return 0;
}