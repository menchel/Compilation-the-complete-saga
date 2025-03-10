%{
/* Declarations section */
#include <stdio.h>
#include <memory>
#include "visitor.hpp"
#include "nodes.hpp"
#include "output.hpp"
#include "parser.tab.h"
%}

%option yylineno
%option noyywrap

whitespace       [\t\n\r ]

%%

void             { return VOID; }
int              { return INT; }
byte             { return BYTE; }
bool             { return BOOL; }
and              { return AND; }
or               { return OR; }
not              { return NOT; }
true             { return TRUE; }
false            { return FALSE; }
return           { return RETURN; }
if               { return IF; }
else             { return ELSE; }
while            { return WHILE; }
break            { return BREAK; }
continue         { return CONTINUE; }
;                { return SC; }
,                { return COMMA; }
\(               { return LPAREN; }
\)               { return RPAREN; }
\{               { return LBRACE; }
\}               { return RBRACE; }
=                { return ASSIGN; }
==               { return EQ; }
!=               { return NE; }
\<                { return LT; }
\>                { return GT; }
\>=               { return GE; }
\<=               { return LE; }
\-               { return SUB; }
\+               { return ADD; }
\/               { return DIV; }
\*               { return MUL; }
[a-zA-Z][a-zA-Z0-9]* { yylval = std::make_shared<ast::ID>((std::string(yytext)).c_str()); return ID; }
0|[1-9][0-9]*        { yylval = std::make_shared<ast::Num>((std::string(yytext)).c_str()); return NUM; }
0b|[1-9][0-9]*b              { yylval = std::make_shared<ast::NumB>((std::string(yytext)).c_str()); return NUM_B; }
\"([^\n\r\"\\]|\\[rnt"\\])+\" 	{ yylval = std::make_shared<ast::String>((std::string(yytext)).c_str()); return STRING; }
\/\/[^\n\r]*[\r\n]? { /*  */ }
{whitespace}        { /*  */ }
.                   {output::errorLex(yylineno); exit(0); }
%%