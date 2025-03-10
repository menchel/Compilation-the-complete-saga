#include "tokens.hpp"
#include "output.hpp"

int main() {
    enum tokentype token;

    // read tokens until the end of file is reached
    while ((token = static_cast<tokentype>(yylex()))) {
        switch(token)
		{
			case VOID:
				output::printToken(yylineno,VOID,yytext);
				break;
			case INT:
				output::printToken(yylineno,INT,yytext);
				break;
			case BYTE:
				output::printToken(yylineno,BYTE,yytext);
				break;
			case BOOL:
				output::printToken(yylineno,BOOL,yytext);
				break;
			case AND:
				output::printToken(yylineno,AND,yytext);
				break;
			case OR:
				output::printToken(yylineno,OR,yytext);
				break;
			case NOT:
				output::printToken(yylineno,NOT,yytext);
				break;
			case TRUE:
				output::printToken(yylineno,TRUE,yytext);
				break;
			case FALSE:
				output::printToken(yylineno,FALSE,yytext);
				break;
			case RETURN:
				output::printToken(yylineno,RETURN,yytext);
				break;
			case IF:
				output::printToken(yylineno,IF,yytext);
				break;
			case ELSE:
				output::printToken(yylineno,ELSE,yytext);
				break;
			case WHILE:
				output::printToken(yylineno,WHILE,yytext);
				break;
			case BREAK:
				output::printToken(yylineno,BREAK,yytext);
				break;
			case CONTINUE:
				output::printToken(yylineno,CONTINUE,yytext);
				break;
			case SC:
				output::printToken(yylineno,SC,yytext);
				break;
			case COMMA:
				output::printToken(yylineno,COMMA,yytext);
				break;
			case LPAREN:
				output::printToken(yylineno,LPAREN,yytext);
				break;
			case RPAREN:
				output::printToken(yylineno,RPAREN,yytext);
				break;
			case LBRACE:
				output::printToken(yylineno,LBRACE,yytext);
				break;
			case RBRACE:
				output::printToken(yylineno,RBRACE,yytext);
				break;
			case ASSIGN:
				output::printToken(yylineno,ASSIGN,yytext);
				break;
			case RELOP:
				output::printToken(yylineno,RELOP,yytext);
				break;
			case BINOP:
				output::printToken(yylineno,BINOP,yytext);
				break;
			case COMMENT:
				output::printToken(yylineno,COMMENT,yytext);
				break;
			case ID:
				output::printToken(yylineno,ID,yytext);
				break;
			case NUM:
				output::printToken(yylineno,NUM,yytext);
				break;
			case NUM_B:
				output::printToken(yylineno,NUM_B,yytext);
				break;
			case STRING:
				output::printString(yylineno,STRING,yytext);
				break;
			case UNCLOSED_STRING:
				output::errorUnclosedString();
				break;
			case UNDEF_ESCAPE:
				output::handleUndefinedEscape(yytext);
				break;
			case UNDEF_ESCAPE_HEX:
				output::handleUndefinedHex(yytext);
				break;
			default:	//ERROR
				output::handleGeneralError(yytext);
				break;
		}
    }
    return 0;
}