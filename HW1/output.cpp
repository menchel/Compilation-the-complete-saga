#include "output.hpp"
#include <iostream>
#include <sstream>

static const std::string token_names[] = {
        "__FILLER_FOR_ZERO",
        "VOID",
        "INT",
        "BYTE",
        "BOOL",
        "AND",
        "OR",
        "NOT",
        "TRUE",
        "FALSE",
        "RETURN",
        "IF",
        "ELSE",
        "WHILE",
        "BREAK",
        "CONTINUE",
        "SC",
        "COMMA",
        "LPAREN",
        "RPAREN",
        "LBRACE",
        "RBRACE",
        "ASSIGN",
        "RELOP",
        "BINOP",
        "COMMENT",
        "ID",
        "NUM",
        "NUM_B",
        "STRING",
		"UNCLOSED_STRING",
		"UNKNOWN_CHAR",
		"UNDEF_ESCAPE",
		"ERROR"
};

void output::printToken(int lineno, enum tokentype token, const char *value) {
    if (token == COMMENT) {
        std::cout << lineno << " COMMENT //" << std::endl;
    }
	else
	{
        std::cout << lineno << " " << token_names[token] << " " << value << std::endl;
    }
}
void output::printString(int lineno, enum tokentype token, const char *value)
{
	std::string txt(value);
	int txtSize=txt.size();
	//start from line and STRING
	std::cout << lineno << " STRING ";
	for(int cur=0;cur<txtSize;cur++)
	{
		char c=txt[cur];
		char next=txt[cur+1];
		if(c=='"') //wihte space or ending
		{
			continue;
		}
		else if(c=='\\' && next=='x') //hex
		{
			//note that hex is valid, otherwise it would have been caught by UNDEFINED_HEX
			std::stringstream stream;
			stream << std::hex << txt.substr(cur+2, 2);//take the two hex chars
            			int numFromHex;
            			stream >> numFromHex;
            			std::cout << char(numFromHex);
           		 	cur=cur+3;
		}
		else if(c == '\\' && next == 'n')
		{
			std::cout << '\n';
			cur++;
		}
		else if(c=='\\' && next=='r')
		{
			std::cout << '\r';
			cur++;
		}
		else if(c=='\\' && next=='t')
		{
			std::cout << '\t';
			cur++;
		}
		else if(c=='\\' && next=='\"')
		{
			std::cout << '\"';
			cur++;
		}
		else if(c=='\\' && next=='\\')
		{
			std::cout << '\\';
			cur++;
		}
		else if(c=='\\' && next=='0') //terminator
		{
			std::cout << std::endl;
			return;
		}
		else if(c=='\\')
		{
			std::cout << '\\';
		}
		else
		{
			std::cout << c;
		}
	}
	std::cout << std::endl;
}

void output::handleUndefinedHex(const char *value)
{
	std::string txt(value);
	int txtSize=txt.size();
	if(txt[txtSize-2]=='x')//hex with 1 characters
	{
		char seq[2]={txt[txtSize-2],'\0'};
		errorUndefinedEscape(seq);
	}
	if(txt[txtSize-1]!='"')
	{
		char seq[4]={'x',txt[txtSize-2],txt[txtSize-1],'\0'};
		errorUndefinedEscape(seq);
	}
	//hex with 2 characters
	char seq[3]={'x',txt[txtSize-2],'\0'};
	errorUndefinedEscape(seq);
	
}
void output::handleUndefinedEscape(const char *value)
{
	std::string txt(value);
	int txtSize=txt.size();
	char seq[2]={txt[txtSize-1],'\0'};
	errorUndefinedEscape(seq);
}
void output::handleGeneralError(const char *value)
{
	//we expect a single value here
	errorUnknownChar(*value);
}
	
void output::errorUnknownChar(char c) {
    std::cout << "ERROR: Unknown character " << c << std::endl;
    exit(0);
}

void output::errorUnclosedString() {
    std::cout << "ERROR: Unclosed string" << std::endl;
    exit(0);
}

void output::errorUndefinedEscape(const char *sequence) {
    std::cout << "ERROR: Undefined escape sequence " << sequence << std::endl;
    exit(0);
}
