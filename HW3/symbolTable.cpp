#include "symbolTable.hpp"

//checks if the symbol exists
bool SymbolTable::isExists(const std::string& symbolName)
{
	for (std::shared_ptr<SingleSymbol> symbol : table)
	{
		if (symbolName == symbol->symbolName)
		{
			return true;
		}
	}
	return false;
}

//adds a new symbol
void SymbolTable::addNewSymbol(std::shared_ptr<SingleSymbol> symbolPtr)
{
	table.push_back(symbolPtr);
	maximumOffsetOfBlock = symbolPtr->symbolOffset;
}

//returns a symbol
std::shared_ptr<SingleSymbol> SymbolTable::getSymbol(const std::string& symbolName)
{
	for (std::shared_ptr<SingleSymbol> symbol : table)
	{
		if (symbolName == symbol->symbolName)
		{
			return symbol;
		}
	}
	return nullptr;
}

StackSymbolTables::StackSymbolTables():tablesStack(),tablesOffsets()
{
	tablesOffsets.push_back(0);
	push(false,false);
	std::vector<std::string> printParameters = { "string" };
	std::vector<std::string> printiParameters = { "int" };
	addSymbol("print", "void", true, printParameters);
	addSymbol("printi", "void", true, printiParameters);
}

//is exists
bool StackSymbolTables::isSymbolExistsInAnyScope(const std::string& name)
{
	for (std::shared_ptr<SymbolTable>currentTable : tablesStack)
	{
		if (currentTable->isExists(name))
		{
			return true;
		}
	}
	return false;
}

//push a new scope
void StackSymbolTables::push(bool isInLoop, bool isFunction, const std::string& returnType)
{
	std::shared_ptr<SymbolTable>newTable;
	if (isFunction)
	{
		newTable = std::make_shared<SymbolTable>(false, 0, returnType);
	}
	else
	{
		newTable = std::make_shared<SymbolTable>(isInLoop, tablesOffsets.back(), returnType);
	}
	tablesOffsets.push_back(newTable->maximumOffsetOfBlock);
	tablesStack.push_back(newTable);
}

//pop
void StackSymbolTables::pop()
{
	tablesStack.pop_back();
	tablesOffsets.pop_back();
}

//current
std::shared_ptr<SymbolTable> StackSymbolTables::getCurrentScope()
{
	return tablesStack.back();
}

//add element
void StackSymbolTables::addSymbol(const std::string& name, const std::string& type, bool isFunction, const std::vector<std::string>functionParameters)
{
	std::shared_ptr<SymbolTable>current = getCurrentScope();
	int symbolOffset = 0;
	if (!isFunction)
	{
		symbolOffset = tablesOffsets.back();
		tablesOffsets.push_back(symbolOffset + 1);
	}
	current->addNewSymbol(std::make_shared<SingleSymbol>(name, symbolOffset, type, isFunction, functionParameters));
}

std::shared_ptr<SingleSymbol> StackSymbolTables::getSymbol(const std::string& symbolName)
{
	for (std::shared_ptr<SymbolTable>currentTable : tablesStack)
	{
		std::shared_ptr<SingleSymbol> currSymbol = currentTable->getSymbol(symbolName);
		if (currSymbol!=nullptr)
		{
			return currSymbol;
		}
	}
	return nullptr;
}