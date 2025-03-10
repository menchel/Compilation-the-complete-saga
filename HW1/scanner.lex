%{

/* Declarations section */
#include <stdio.h>
#include "tokens.hpp"

%}

%option yylineno
%option noyywrap
%x STR
digit   		([0-9])
letter  		([a-zA-Z])
idchar		([0-9a-zA-Z])
whitespace		([\t\n\r ])
nocomment		([^\n\r])
printablehex	(\\x([2-6][0-9a-fA-F]|7[0-9a-eA-E]|0[DdAa9]))
escape			([\\ntr\"0])
printable	([ !#-\[\]-~]|\\{escape}|{printablehex})
badhex	(\\x([^0-7][0-9a-fA-F]|[0-9A-Fa-f][^0-9A-Fa-f]|1[0-9A-Fa-f]|0[0-8BbCceEFf]|7f|7F|[89A-Fa-f][0-9A-Fa-f]|[^0-7][^0-9A-Fa-f]|[^0-9A-Fa-f]))
%%

void			return VOID;
int				return INT;
byte			return BYTE;
bool			return BOOL;
and				return AND;
or				return OR;
not				return NOT;
true			return TRUE;
false			return FALSE;
return			return RETURN;
if				return IF;
else			return ELSE;
while			return WHILE;
break			return BREAK;
continue		return CONTINUE;
;				return SC;
,				return COMMA;
\(				return LPAREN;
\)				return RPAREN;
\{				return LBRACE;
\}				return RBRACE;
=				return ASSIGN;
[<>=!]=|<|>		return RELOP;
\+|\-|\*|\/			return BINOP;
\/\/{nocomment}*  			return COMMENT;
{letter}{idchar}*				return ID;
0|[1-9]{digit}*				return NUM;
0b|[1-9]{digit}*b				return NUM_B;
\"({printable})*\"				return STRING;
\"({printable})*				return UNCLOSED_STRING;
\"({printable})*{badhex}				return UNDEF_ESCAPE_HEX;
\"({printable})*\\[^\\ntr\"0]				return UNDEF_ESCAPE;
{whitespace}	;
.				return ERROR;
%%

