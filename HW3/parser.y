%{
#include <memory>
#include "nodes.hpp"
#include "output.hpp"
#include "symbolTable.hpp"

// bison declarations
extern int yylineno;
extern int yylex();

void yyerror(const char*);

// root of the AST, set by the parser and used by other parts of the compiler
std::shared_ptr<ast::Node> program;

using namespace std;

// TODO: Place any additional declarations here
%}

%token VOID INT BYTE BOOL TRUE FALSE RETURN IF WHILE BREAK CONTINUE SC COMMA ID NUM NUM_B STRING

// TODO: Define precedence and associativity here

%right ASSIGN
%left OR
%left AND
%left NE EQ GE LE GT LT
%left ADD SUB
%left MUL DIV
%right NOT
%left LBRACE RBRACE LPAREN RPAREN
%right ELSE

%%

// While reducing the start variable, set the root of the AST
Program:  Funcs { program = $1; }

// TODO: Define grammar here
Funcs:	/*epsilon*/	{$$=std::make_shared<ast::Funcs>();} 
	|FuncDecl Funcs	{(std::dynamic_pointer_cast<ast::Funcs>($2))->push_front(std::dynamic_pointer_cast<ast::FuncDecl>($1));  $$=$2;}

FuncDecl:RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE {$$=std::make_shared<ast::FuncDecl>((std::dynamic_pointer_cast<ast::ID>($2)),(std::dynamic_pointer_cast<ast::Type>($1)),(std::dynamic_pointer_cast<ast::Formals>($4)),(std::dynamic_pointer_cast<ast::Statements>($7)));}

RetType:Type {$$=$1;}
	|VOID	{$$=std::make_shared<ast::Type>(ast::BuiltInType::VOID);}

Formals: /*epsilon*/ {$$=std::make_shared<ast::Formals>();}
	| FormalsList	{$$=$1;}

FormalsList: FormalDecl	{$$=std::make_shared<ast::Formals>(std::dynamic_pointer_cast<ast::Formal>($1));}
	| FormalDecl COMMA FormalsList {(std::dynamic_pointer_cast<ast::Formals>($3))->push_front(std::dynamic_pointer_cast<ast::Formal>($1)); $$=$3;}

FormalDecl: Type ID	{$$=std::make_shared<ast::Formal>((std::dynamic_pointer_cast<ast::ID>($2)),(std::dynamic_pointer_cast<ast::Type>($1)));}

Statements: Statement {$$=std::make_shared<ast::Statements>(std::dynamic_pointer_cast<ast::Statement>($1));}
	| Statements Statement	{(std::dynamic_pointer_cast<ast::Statements>($1))->push_back(std::dynamic_pointer_cast<ast::Statement>($2));  $$=$1;}

Statement: LBRACE Statements RBRACE {(std::dynamic_pointer_cast<ast::Statement>($2))->hasParen=true; $$=$2;}
	| Type ID SC	{$$=std::make_shared<ast::VarDecl>((std::dynamic_pointer_cast<ast::ID>($2)),(std::dynamic_pointer_cast<ast::Type>($1)));}
	| Type ID ASSIGN Exp SC	{$$=std::make_shared<ast::VarDecl>((std::dynamic_pointer_cast<ast::ID>($2)),(std::dynamic_pointer_cast<ast::Type>($1)),(std::dynamic_pointer_cast<ast::Exp>($4)));}
	| ID ASSIGN Exp SC	{$$=std::make_shared<ast::Assign>((std::dynamic_pointer_cast<ast::ID>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)));}
	| Call SC	{$$=$1;}
	| RETURN SC	{$$=std::make_shared<ast::Return>();}
	| RETURN Exp SC	{$$=std::make_shared<ast::Return>(std::dynamic_pointer_cast<ast::Exp>($2));}
	| IF LPAREN Exp RPAREN Statement {$$=std::make_shared<ast::If>((std::dynamic_pointer_cast<ast::Exp>($3)),(std::dynamic_pointer_cast<ast::Statement>($5)));}
	| IF LPAREN Exp RPAREN Statement ELSE Statement	 {$$=std::make_shared<ast::If>((std::dynamic_pointer_cast<ast::Exp>($3)),(std::dynamic_pointer_cast<ast::Statement>($5)),(std::dynamic_pointer_cast<ast::Statement>($7)));}
	| WHILE LPAREN Exp RPAREN Statement	{$$=std::make_shared<ast::While>((std::dynamic_pointer_cast<ast::Exp>($3)),(std::dynamic_pointer_cast<ast::Statement>($5)));}
	| BREAK SC	{$$=std::make_shared<ast::Break>();}
	| CONTINUE SC	{$$=std::make_shared<ast::Continue>();}

Call: ID LPAREN ExpList RPAREN	{$$=std::make_shared<ast::Call>((std::dynamic_pointer_cast<ast::ID>($1)),(std::dynamic_pointer_cast<ast::ExpList>($3)));}
	| ID LPAREN RPAREN	{$$=std::make_shared<ast::Call>((std::dynamic_pointer_cast<ast::ID>($1)));}

ExpList: Exp	{$$=std::make_shared<ast::ExpList>((std::dynamic_pointer_cast<ast::Exp>($1)));}
	| Exp COMMA ExpList	{(std::dynamic_pointer_cast<ast::ExpList>($3))->push_front(std::dynamic_pointer_cast<ast::Exp>($1)); $$=$3;}

Type: INT	{$$=std::make_shared<ast::Type>(ast::BuiltInType::INT);}
	| BYTE	{$$=std::make_shared<ast::Type>(ast::BuiltInType::BYTE);}
	| BOOL	{$$=std::make_shared<ast::Type>(ast::BuiltInType::BOOL);}

Exp: LPAREN Exp RPAREN	{$$=std::dynamic_pointer_cast<ast::Exp>($2);}
	| Exp SUB Exp	{$$=std::make_shared<ast::BinOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::BinOpType::SUB);}
	| Exp ADD Exp	{$$=std::make_shared<ast::BinOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::BinOpType::ADD);}
	| Exp DIV Exp	{$$=std::make_shared<ast::BinOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::BinOpType::DIV);}
	| Exp MUL Exp	{$$=std::make_shared<ast::BinOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::BinOpType::MUL);}
	| ID	{$$=std::dynamic_pointer_cast<ast::ID>($1);}
	| Call	{$$=$1;}
	| NUM	{$$=std::dynamic_pointer_cast<ast::Num>($1);}
	| NUM_B	{$$=std::dynamic_pointer_cast<ast::NumB>($1);}
	| STRING	{$$=std::dynamic_pointer_cast<ast::String>($1);}
	| TRUE	{$$=std::make_shared<ast::Bool>(true);}
	| FALSE	{$$=std::make_shared<ast::Bool>(false);}
	| NOT Exp	{$$=std::make_shared<ast::Not>((std::dynamic_pointer_cast<ast::Exp>($2)));}
	| Exp AND Exp	{$$=std::make_shared<ast::And>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)));}
	| Exp OR Exp	{$$=std::make_shared<ast::Or>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)));}
	| Exp EQ Exp	{$$=std::make_shared<ast::RelOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::RelOpType::EQ);}
	| Exp NE Exp	{$$=std::make_shared<ast::RelOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::RelOpType::NE);}
	| Exp LT Exp	{$$=std::make_shared<ast::RelOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::RelOpType::LT);}
	| Exp GT Exp	{$$=std::make_shared<ast::RelOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::RelOpType::GT);}
	| Exp LE Exp	{$$=std::make_shared<ast::RelOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::RelOpType::LE);}
	| Exp GE Exp	{$$=std::make_shared<ast::RelOp>((std::dynamic_pointer_cast<ast::Exp>($1)),(std::dynamic_pointer_cast<ast::Exp>($3)),ast::RelOpType::GE);}
	| LPAREN Type RPAREN Exp	{$$=std::make_shared<ast::Cast>((std::dynamic_pointer_cast<ast::Exp>($4)),(std::dynamic_pointer_cast<ast::Type>($2)));}



%%
int main()
{
	yyparse();
	output::SemanticVisitor visitor;
	program->accept(visitor);
	visitor.printEnd();
}

void yyerror(const char * message){
	output::errorSyn(yylineno);
	exit(0);
}
// TODO: Place any additional code here