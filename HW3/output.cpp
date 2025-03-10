#include "output.hpp"
#include <iostream>

namespace output {
    /* Helper functions */

    static std::string toString(ast::BuiltInType type) {
        switch (type) {
        case ast::BuiltInType::INT:
            return "int";
        case ast::BuiltInType::BOOL:
            return "bool";
        case ast::BuiltInType::BYTE:
            return "byte";
        case ast::BuiltInType::VOID:
            return "void";
        case ast::BuiltInType::STRING:
            return "string";
        default:
            return "unknown";
        }
    }
        static std::vector<std::string> convertToBigLetters(std::vector<std::string>samllLetters) {
            std::vector<std::string> res;
            for (std::string type : samllLetters)
            {
                if (type == "int")
                {
                    res.push_back("INT");
                }
                else if(type == "bool")
                {
                    res.push_back("BOOL");
                }
                else if (type == "byte")
                {
                    res.push_back("BYTE");
                }
                else if (type == "void")
                {
                    res.push_back("VOID");
                }
                else if (type == "string")
                {
                    res.push_back("STRING");
                }
                else
                {
                    res.push_back("UNKNOWN");
                }
            }
            return res;
        }

    /* Error handling functions */

    void errorLex(int lineno) {
        std::cout << "line " << lineno << ": lexical error\n";
        exit(0);
    }

    void errorSyn(int lineno) {
        std::cout << "line " << lineno << ": syntax error\n";
        exit(0);
    }

    void errorUndef(int lineno, const std::string &id) {
        std::cout << "line " << lineno << ":" << " variable " << id << " is not defined" << std::endl;
        exit(0);
    }

    void errorDefAsFunc(int lineno, const std::string &id) {
        std::cout << "line " << lineno << ":" << " symbol " << id << " is a function" << std::endl;
        exit(0);
    }

    void errorDefAsVar(int lineno, const std::string &id) {
        std::cout << "line " << lineno << ":" << " symbol " << id << " is a variable" << std::endl;
        exit(0);
    }

    void errorDef(int lineno, const std::string &id) {
        std::cout << "line " << lineno << ":" << " symbol " << id << " is already defined" << std::endl;
        exit(0);
    }

    void errorUndefFunc(int lineno, const std::string &id) {
        std::cout << "line " << lineno << ":" << " function " << id << " is not defined" << std::endl;
        exit(0);
    }

    void errorMismatch(int lineno) {
        std::cout << "line " << lineno << ":" << " type mismatch" << std::endl;
        exit(0);
    }

    void errorPrototypeMismatch(int lineno, const std::string &id, std::vector<std::string> &paramTypes) {
        std::cout << "line " << lineno << ": prototype mismatch, function " << id << " expects parameters (";

        for (int i = 0; i < paramTypes.size(); ++i) {
            std::cout << paramTypes[i];
            if (i != paramTypes.size() - 1)
                std::cout << ",";
        }

        std::cout << ")" << std::endl;
        exit(0);
    }

    void errorUnexpectedBreak(int lineno) {
        std::cout << "line " << lineno << ":" << " unexpected break statement" << std::endl;
        exit(0);
    }

    void errorUnexpectedContinue(int lineno) {
        std::cout << "line " << lineno << ":" << " unexpected continue statement" << std::endl;
        exit(0);
    }

    void errorMainMissing() {
        std::cout << "Program has no 'void main()' function" << std::endl;
        exit(0);
    }

    void errorByteTooLarge(int lineno, const int value) {
        std::cout << "line " << lineno << ": byte value " << value << " out of range" << std::endl;
        exit(0);
    }

    /* ScopePrinter class */

    ScopePrinter::ScopePrinter() : indentLevel(0) {}

    std::string ScopePrinter::indent() const {
        std::string result;
        for (int i = 0; i < indentLevel; ++i) {
            result += "  ";
        }
        return result;
    }

    void ScopePrinter::beginScope() {
        indentLevel++;
        buffer << indent() << "---begin scope---" << std::endl;
    }

    void ScopePrinter::endScope() {
        buffer << indent() << "---end scope---" << std::endl;
        indentLevel--;
    }

    void ScopePrinter::emitVar(const std::string &id, const ast::BuiltInType &type, int offset) {
        buffer << indent() << id << " " << toString(type) << " " << offset << std::endl;
    }

    void ScopePrinter::emitFunc(const std::string &id, const ast::BuiltInType &returnType,
                                const std::vector<ast::BuiltInType> &paramTypes) {
        globalsBuffer << id << " " << "(";

        for (int i = 0; i < paramTypes.size(); ++i) {
            globalsBuffer << toString(paramTypes[i]);
            if (i != paramTypes.size() - 1)
                globalsBuffer << ",";
        }

        globalsBuffer << ")" << " -> " << toString(returnType) << std::endl;
    }

    std::ostream &operator<<(std::ostream &os, const ScopePrinter &printer) {
        os << "---begin global scope---" << std::endl;
        os << printer.globalsBuffer.str();
        os << printer.buffer.str();
        os << "---end global scope---" << std::endl;
        return os;
    }

    SemanticVisitor::SemanticVisitor() :printer(), stack()
    {

    }

    void SemanticVisitor::visit(ast::Num& node)
    {
        node.expType = "int";
    }

    void SemanticVisitor::visit(ast::NumB& node)
    {
        if (node.value > 255)
        {
            output::errorByteTooLarge(node.line, node.value);
        }
        node.expType = "byte";
    }

    void SemanticVisitor::visit(ast::String& node)
    {
        node.expType = "string";
    }

    void SemanticVisitor::visit(ast::Bool& node)
    {
        node.expType = "bool";
    }

    void SemanticVisitor::visit(ast::ID& node)
    {
        std::shared_ptr<SingleSymbol>symbol = stack.getSymbol(node.value);
        if (symbol != nullptr)
        {
            node.expType = symbol->type;
        }
        else
        {
            output::errorUndef(node.line, node.value);
        }
    }

    void SemanticVisitor::visit(ast::BinOp& node)
    {
        node.left->accept(*this);
        node.right->accept(*this);
        if (isFunction(node.left))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.left)->value);
        }
        if (isFunction(node.right))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.right)->value);
        }
        std::string leftType = node.left->expType;
        std::string rightType = node.right->expType;

        if ((leftType == "int" && rightType == "int") || (leftType == "int" && rightType == "byte") || (leftType == "byte" && rightType == "int"))
        {
            node.expType = "int";
        }
        else
        {
            if (leftType == "byte" && rightType == "byte")
            {
                node.expType = "byte";
            }
            else
            {
                output::errorMismatch(node.line);
            }
        }
    }

    void SemanticVisitor::visit(ast::RelOp& node)
    {
        node.left->accept(*this);
        node.right->accept(*this);
        if (isFunction(node.left))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.left)->value);
        }
        if (isFunction(node.right))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.right)->value);
        }
        std::string leftType = node.left->expType;
        std::string rightType = node.right->expType;
        if ((leftType == "int" && rightType == "int") || (leftType == "int" && rightType == "byte") || (leftType == "byte" && rightType == "int"))
        {
            node.expType = "bool";
        }
        else
        {
            if (leftType == "byte" && rightType == "byte")
            {
                node.expType = "bool";
            }
            else
            {
                output::errorMismatch(node.line);
            }
        }
    }

    void SemanticVisitor::visit(ast::Not& node)
    {
        node.exp->accept(*this);
        if (isFunction(node.exp))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.exp)->value);
        }
        if (node.exp->expType != "bool")
        {
            output::errorMismatch(node.line);
        }
        node.expType = "bool";
    }

    void SemanticVisitor::visit(ast::And& node)
    {
        node.left->accept(*this);
        node.right->accept(*this);
        if (isFunction(node.left))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.left)->value);
        }
        if (isFunction(node.right))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.right)->value);
        }
        if (!(node.left->expType == "bool" && node.right->expType == "bool"))
        {
            output::errorMismatch(node.line);
        }
        node.expType = "bool";
    }

    void SemanticVisitor::visit(ast::Or& node)
    {
        node.left->accept(*this);
        node.right->accept(*this);
        if (isFunction(node.left))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.left)->value);
        }
        if (isFunction(node.right))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.right)->value);
        }
        if (!(node.left->expType == "bool" && node.right->expType == "bool"))
        {
            output::errorMismatch(node.line);
        }
        node.expType = "bool";
    }

    void SemanticVisitor::visit(ast::Type& node)
    {
        
    }

    void SemanticVisitor::visit(ast::Cast& node)
    {
        node.exp->accept(*this);
        if (isFunction(node.exp))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.exp)->value);
        }
        node.target_type->accept(*this);
        if (toString(node.target_type->type) == "int" && (node.exp->expType == "int" || node.exp->expType == "byte"))
        {
            node.expType = "int";
            return;
        }
        if (toString(node.target_type->type) == "byte" && (node.exp->expType == "int" || node.exp->expType == "byte"))
        {
            node.expType = "byte"; 
            return;
        }
        output::errorMismatch(node.line);
    }

    void SemanticVisitor::visit(ast::ExpList& node)
    {
        for (std::shared_ptr<ast::Exp> exp : node.exps)
        {
            exp->accept(*this);
        }
    }

    void SemanticVisitor::visit(ast::Call& node)
    {
        std::shared_ptr<SingleSymbol>func = stack.getSymbol(node.func_id->value);
        if (func == nullptr) //doesn't exists
        {
            output::errorUndefFunc(node.line, node.func_id->value);
        }
        if (!func->isFunction)
        {
            output::errorDefAsVar(node.line, node.func_id->value);
        }
        node.args->accept(*this); // make sure that the elements give are OK
        std::vector<std::shared_ptr<ast::Exp>> parametersGiven = node.args->exps;
        std::vector<std::string> parametersExpected = func->functionParameters;
        std::vector<std::string> parametersExpectedForOutput = convertToBigLetters(parametersExpected);
        if (parametersGiven.size() != parametersExpected.size())
        {
            output::errorPrototypeMismatch(node.line, node.func_id->value, parametersExpectedForOutput);
        }
        int size = parametersGiven.size();
        for (int i = 0; i < size; i++)
        {
            if (parametersGiven[i]->expType != parametersExpected[i])
            {
                if (parametersExpected[i] == "int" && parametersGiven[i]->expType == "byte")
                {
                    continue;
                }
                output::errorPrototypeMismatch(node.line, node.func_id->value, parametersExpectedForOutput);
            }
        }
        node.expType = func->type;
    }

    void SemanticVisitor::visit(ast::Statements& node)
    {
        bool is_loop = stack.getCurrentScope()->isScopeOfLoop;
        if (node.hasParen)
        {
            printer.beginScope();
            stack.push(is_loop, false, stack.getCurrentScope()->typeOfReturn);
        }
        std::vector<std::shared_ptr<ast::Statement>> elems = node.statements;
        for (std::shared_ptr<ast::Statement> st : elems)
        {
            st->accept(*this);
        }
        if (node.hasParen)
        {
            stack.pop();
            printer.endScope();
        }
    }

    void SemanticVisitor::visit(ast::Break& node)
    {
        if (!stack.getCurrentScope()->isScopeOfLoop)
        {
            output::errorUnexpectedBreak(node.line);
        }
    }

    void SemanticVisitor::visit(ast::Continue& node)
    {
        if (!stack.getCurrentScope()->isScopeOfLoop)
        {
            output::errorUnexpectedContinue(node.line);
        }
    }

    void SemanticVisitor::visit(ast::Return& node)
    {
        if (node.exp == nullptr) //return void
        {
            if (stack.getCurrentScope()->typeOfReturn != "void")
            {
                output::errorMismatch(node.line);
            }
            return;
        }
        node.exp->accept(*this);
        if (isFunction(node.exp))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.exp)->value);
        }
        if (node.exp->expType != stack.getCurrentScope()->typeOfReturn)
        {
            if (node.exp->expType == "byte" && stack.getCurrentScope()->typeOfReturn == "int")
            {
                return;
            }
            output::errorMismatch(node.line);
        }
    }

    void SemanticVisitor::visit(ast::If& node)
    {
        printer.beginScope();
        stack.push(stack.getCurrentScope()->isScopeOfLoop,false, stack.getCurrentScope()->typeOfReturn);
        node.condition->accept(*this);

        if (node.condition->expType != "bool")
        {
            output::errorMismatch(node.condition->line);
        }
        node.then->accept(*this);

        stack.pop();
        printer.endScope();

        if (node.otherwise != nullptr) // there is an else
        {
            printer.beginScope();
            stack.push(stack.getCurrentScope()->isScopeOfLoop, false, stack.getCurrentScope()->typeOfReturn);

            node.otherwise->accept(*this);

            stack.pop();
            printer.endScope();
        }
    }

    void SemanticVisitor::visit(ast::While& node)
    {
        printer.beginScope();
        stack.push(true,false, stack.getCurrentScope()->typeOfReturn);
        node.condition->accept(*this);

        if (node.condition->expType != "bool")
        {
            output::errorMismatch(node.condition->line);
        }
        node.body->accept(*this);
        printer.endScope();
        stack.pop();
    }

    void SemanticVisitor::visit(ast::VarDecl& node)
    {
        node.type->accept(*this);
        if (stack.isSymbolExistsInAnyScope(node.id->value))
        {
            output::errorDef(node.line, node.id->value);
        }
        if (node.init_exp != nullptr) //initial
        {
            node.init_exp->accept(*this);
        }
        stack.addSymbol(node.id->value, toString(node.type->type), false);
        std::shared_ptr<SingleSymbol> var = stack.getSymbol(node.id->value);
        printer.emitVar(var->symbolName, node.type->type, var->symbolOffset);
        if (node.init_exp != nullptr) //initial
        {
            if (isFunction(node.init_exp))
            {
                output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.init_exp)->value);
            }
            if (var->type != node.init_exp->expType)
            {
                if (!(var->type == "int" && node.init_exp->expType == "byte"))
                {
                    output::errorMismatch(node.line);
                }
            }
        }
    }

    void SemanticVisitor::visit(ast::Assign& node)
    {
        node.id->accept(*this);
        node.exp->accept(*this);
        if (isFunction(node.exp))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.exp)->value);
        }
        if (isFunction(node.id))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.id)->value);
        }
        std::shared_ptr<SingleSymbol> var = stack.getSymbol(node.id->value);
        if (var->type != node.exp->expType)
        {
            if (!(var->type == "int" && node.exp->expType == "byte"))
            {
                output::errorMismatch(node.line);
            }
        }
    }

    void SemanticVisitor::visit(ast::Formal& node)
    {
        node.type->accept(*this);
        stack.addSymbol(node.id->value, toString(node.type->type), false);
    }

    void SemanticVisitor::visit(ast::Formals& node)
    {
        std::vector<std::shared_ptr<ast::Formal>>& vec = node.formals;
        int counter = -1;
        for (std::shared_ptr<ast::Formal>formal : vec)
        {
            formal->accept(*this);
            stack.tablesOffsets.pop_back();
            //changing the offsets
            std::shared_ptr<SingleSymbol>symbol = stack.getSymbol(formal->id->value);
            symbol->symbolOffset = counter;
            printer.emitVar(formal->id->value, formal->type->type, symbol->symbolOffset);
            counter--;
        }
    }

    void SemanticVisitor::visit(ast::FuncDecl& node)
    {
        //here, the defenition already was made and types of formals were checked
        //we only need to enter the scope
        printer.beginScope();
        stack.push(false,true, toString(node.return_type->type));
        node.formals->accept(*this);
        node.body->accept(*this);
        stack.pop();
        printer.endScope();
    }

    void SemanticVisitor::visit(ast::Funcs& node)
    {
        std::vector<std::shared_ptr<ast::FuncDecl>>funcs = node.funcs;
        //enter all defenitions to global scope
        printer.emitFunc("print", ast::BuiltInType::VOID, { ast::BuiltInType::STRING });
        printer.emitFunc("printi", ast::BuiltInType::VOID, { ast::BuiltInType::INT });
        //need to make sure
        // 1) main exists
        // 2) no two functions with the same name
        // 3) no two variables from the same function with the same name
        bool mainExists = false;
        bool mainHere = false;
        for (std::shared_ptr<ast::FuncDecl>function:funcs)
        {
            std::string name = function->id->value;
            if (stack.isSymbolExistsInAnyScope(name)) //already exists
            {
                output::errorDef(function->id->line, name);
            }
            if (name == "main")
            {
                mainExists = true;
                mainHere = true;
            }
            std::vector<std::string>formalNames;
            std::vector<std::string>types;
            std::vector<ast::BuiltInType>typesAst;
            std::vector<std::shared_ptr<ast::Formal>>& params = function->formals->formals;
            for (std::shared_ptr<ast::Formal> param : params)
            {
                if (stack.isSymbolExistsInAnyScope(param->id->value) || param->id->value == name)
                {
                    output::errorDef(param->line, param->id->value);
                }
                for (std::string current : formalNames)
                {
                    if (current == param->id->value)
                    {
                        output::errorDef(param->line, current);
                    }
                }
                formalNames.push_back(param->id->value);
                types.push_back(toString(param->type->type));
                typesAst.push_back(param->type->type);
            }
            stack.addSymbol(name, toString(function->return_type->type), true, types);
            printer.emitFunc(name, function->return_type->type, typesAst);
            if (mainHere)
            {
                if (formalNames.size() != 0 || toString(function->return_type->type)!="void")
                {
                    output::errorMainMissing();
                }
                mainHere = false;
            }
        }
        if (!mainExists)
        {
            output::errorMainMissing();
        }
        //actually make the actions
        for (std::shared_ptr<ast::FuncDecl>function : funcs)
        {
            function->accept(*this);
        }
    }

    bool SemanticVisitor::isFunction(std::shared_ptr<ast::Exp> node)
    {
        std::shared_ptr<ast::ID> id = std::dynamic_pointer_cast<ast::ID>(node);
        if (id == nullptr)
        {
            return false;
        }
        std::shared_ptr<SingleSymbol>symbol = stack.getSymbol(id->value);
        if (symbol == nullptr)
        {
            return false;
        }
        return symbol->isFunction;
    }
    void SemanticVisitor::printEnd()
    {
        std::cout << printer;
    }
}
