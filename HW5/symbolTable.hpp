#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <memory>
#include <vector>
#include <string>

struct SingleSymbol
{
	std::string symbolName;
	int symbolOffset;
	std::string type;
	bool isFunction;
	std::vector<std::string>functionParameters;

	SingleSymbol(std::string symbolName, int symbolOffset, std::string type, bool isFunction,const std::vector<std::string>functionParameters=std::vector<std::string>()):symbolName(symbolName), symbolOffset(symbolOffset), type(type),isFunction(isFunction),functionParameters(functionParameters)
	{

	}

	~SingleSymbol() = default;
};

struct SymbolTable
{
	std::vector<std::shared_ptr<SingleSymbol>> table;
	bool isScopeOfLoop;	//for loops
	int maximumOffsetOfBlock;
	std::string typeOfReturn;	//for functions
	std::string baseRegister;

	SymbolTable(bool isScopeOfLoop, int maximumOffsetOfBlock, std::string typeOfReturn = "") :table(), isScopeOfLoop(isScopeOfLoop), maximumOffsetOfBlock(maximumOffsetOfBlock), typeOfReturn(typeOfReturn),baseRegister()
	{

	}

	~SymbolTable() = default;

	//checks if the symbol exists
	bool isExists(const std::string& symbolName);

	//adds a new symbol
	void addNewSymbol(std::shared_ptr<SingleSymbol> symbolPtr);

	//returns a symbol
	std::shared_ptr<SingleSymbol> getSymbol(const std::string& symbolName);

};

struct StackSymbolTables
{
	std::vector<std::shared_ptr<SymbolTable>> tablesStack;
	std::vector<int> tablesOffsets;

	StackSymbolTables();

	~StackSymbolTables() = default;

	//is exists
	bool isSymbolExistsInAnyScope(const std::string& name);

	//push a new scope
	void push(bool isInLoop, bool isFunction, const std::string& returnType="");

	//pop
	void pop();

	//current
	std::shared_ptr<SymbolTable> getCurrentScope();

	//add element
	void addSymbol(const std::string& name,const std::string& type, bool isFunction, const std::vector<std::string>functionParameters = std::vector<std::string>());

	//returns a symbol
	std::shared_ptr<SingleSymbol> getSymbol(const std::string& symbolName);

};
#endif // SYMBOL_TABLE_H

