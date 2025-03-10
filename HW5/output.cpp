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
            else if (type == "bool")
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

    /* CodeBuffer class */

    CodeBuffer::CodeBuffer() : labelCount(0), varCount(0), stringCount(0) {}

    std::string CodeBuffer::freshLabel() {
        return "%label_" + std::to_string(labelCount++);
    }

    std::string CodeBuffer::freshVar() {
        return "%t" + std::to_string(varCount++);
    }

    std::string CodeBuffer::emitString(const std::string &str) {
        std::string var = "@.str" + std::to_string(stringCount++);
        globalsBuffer << var << " = constant [" << str.length() + 1 << " x i8] c\"" << str << "\\00\"" << std::endl;
        return var;
    }

    void CodeBuffer::emit(const std::string &str) {
        buffer << str << std::endl;
    }

    void CodeBuffer::emitLabel(const std::string &label) {
        buffer << label.substr(1) << ":" << std::endl;
    }

    CodeBuffer &CodeBuffer::operator<<(std::ostream &(*manip)(std::ostream &)) {
        buffer << manip;
        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const CodeBuffer &buffer) {
        os << buffer.globalsBuffer.str() << std::endl << buffer.buffer.str();
        return os;
    }

    CodeGeneratorVisitor::CodeGeneratorVisitor() :code(), stack(), checkingBoolForCall(false)
    {
        //create some global defenitions
        //libary function and prints
        code.emit("declare i32 @scanf(i8*, ...)");
        code.emit("declare i32 @printf(i8*, ...)");
        code.emit("declare void @exit(i32)");
        code.emit("@.int_specifier_scan = constant [3 x i8] c\"%d\\00\"");
        code.emit("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
        code.emit("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
        code.emit("@.DIVISION_ERROR = internal constant [23 x i8] c\"Error division by zero\\00\"");
        code.emit("");
        code.emit("define i32 @readi(i32) {");
        code.emit("    %ret_val = alloca i32");
        code.emit("    %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0");
        code.emit("    call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)");
        code.emit("    %val = load i32, i32* %ret_val");
        code.emit("    ret i32 %val");
        code.emit("}");
        code.emit("");
        code.emit("define void @printi(i32) {");
        code.emit("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
        code.emit("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
        code.emit("    ret void");
        code.emit("}");
        code.emit("");
        code.emit("define void @print(i8*) {");
        code.emit("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
        code.emit("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
        code.emit("    ret void");
        code.emit("}");
        //divide by zero
        code.emit("define void @check_div_by_0(i32) {");
        code.emit("%okay = icmp eq i32 %0, 0");
        code.emit("br i1 %okay, label %NO, label %YES");
        code.emit("NO:");
        code.emit("call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIVISION_ERROR, i32 0, i32 0))");
        code.emit("call void @exit(i32 0)");
        code.emit("ret void");
        code.emit("YES:");
        code.emit("ret void");
        code.emit("}");
    }

    void CodeGeneratorVisitor::visit(ast::Num& node)
    {
        node.expType = "int";
        node.regName = code.freshVar();
        code.emit(node.regName + " = add i32 " + std::to_string(node.value) + ", 0");
    }

    void CodeGeneratorVisitor::visit(ast::NumB& node)
    {
        if (node.value > 255)
        {
            output::errorByteTooLarge(node.line, node.value);
        }
        node.expType = "byte";
        node.regName = code.freshVar();
        code.emit(node.regName + " = add i32 " + std::to_string(node.value) + ",0");
    }

    void CodeGeneratorVisitor::visit(ast::String& node)
    {
        node.expType = "string";
        std::string str = node.value;
        std::string tempPlaceString = code.emitString(node.value);
        std::string size = "[" + std::to_string(str.length()+1) + " x i8]";
        std::string getelementptrHelper = "getelementptr" + size + ", " + size + "* " + tempPlaceString + ", i32 0, i32 0";
        node.regName = code.freshVar();
        code.emit(node.regName +" = " + getelementptrHelper);
    }

    void CodeGeneratorVisitor::visit(ast::Bool& node)
    {
        node.expType = "bool";
        node.regName = code.freshVar();
        if (node.value)
        {
            code.emit(node.regName + " = add i32 0, 1");
        }
        else
        {
            code.emit(node.regName + " = add i32 0, 0");
        }
        if (node.falseLabel != "")
        {
            if (node.value)
            {
                code.emit("br label " + node.trueLabel);
            }
            else
            {
                code.emit("br label " + node.falseLabel);
            }
        }
    }

    void CodeGeneratorVisitor::visit(ast::ID& node)
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
        if (symbol->symbolOffset < 0)
        {
            node.regName = "%" + std::to_string(((-1) * symbol->symbolOffset - 1));
        }
        else
        {
            node.regName = loadFromStack(stack.getCurrentScope()->baseRegister, symbol->symbolOffset);
        }

        //bool requires a special treatment
        if (node.expType == "bool" && (!checkingBoolForCall))
        {
            std::string compareResult = code.freshVar();
            code.emit(compareResult + " = icmp eq i32 1, " + node.regName);
            code.emit("br i1 " + compareResult + ", label " + node.trueLabel + " , label " + node.falseLabel);
        }
    }

    void CodeGeneratorVisitor::visit(ast::BinOp& node)
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
        //now create the code
        node.regName = code.freshVar();
        std::string op = "add";
        //get the operation
        if (node.op == ast::BinOpType::ADD)
        {
            op = "add";
        }
        else
        {
            if (node.op == ast::BinOpType::SUB)
            {
                op = "sub";
            }
            else
            {
                if (node.op == ast::BinOpType::MUL)
                {
                    op = "mul";
                }
                else
                {
                    //division depends on the type
                    if (node.expType == "int")
                    {
                        op = "sdiv";
                    }
                    else
                    {
                        op = "udiv";
                    }
                }
            }
        }
        if (node.op == ast::BinOpType::DIV)
        {
            //check division by zero
            code.emit("call void @check_div_by_0(i32 " + node.right->regName + ")");
            code.emit(node.regName + " = " + op + " i32 " + node.left->regName + " , " + node.right->regName);
        }
        else
        {
            code.emit(node.regName + " = " + op + " i32 " + node.left->regName + " , " + node.right->regName);
            //need to make sure that it is okay for byte
            if (node.expType == "byte")
            {
                std::string beforeReg = node.regName;
                node.regName = code.freshVar();
                code.emit(node.regName + " = and i32 255, " + beforeReg);
            }
        }
    }

    void CodeGeneratorVisitor::visit(ast::RelOp& node)
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
        // now generate the code
        node.regName = code.freshVar();
        std::string op = "eq";
        if (node.op == ast::RelOpType::EQ)
        {
            op = "eq";
        }
        else
        {
            if (node.op == ast::RelOpType::NE)
            {
                op = "ne";
            }
            else
            {
                if (node.op == ast::RelOpType::LT)
                {
                    op = "slt";
                }
                else
                {
                    if (node.op == ast::RelOpType::GT)
                    {
                        op = "sgt";
                    }
                    else
                    {
                        if (node.op == ast::RelOpType::LE)
                        {
                            op = "sle";
                        }
                        else
                        {
                            op = "sge";
                        }
                    }
                }
            }
        }
        //now, create
        if (checkingBoolForCall)
        {
            //special case, need to return normal bool
            std::string checkRegister = code.freshVar();
            std::string assignPhi = code.freshLabel();
            std::string trueRegister = code.freshVar();
            std::string falseRegister = code.freshVar();
            std::string labelTrue = code.freshLabel();
            std::string labelfalse = code.freshLabel();
            code.emit(checkRegister + " =icmp " + op + " i32 " + node.left->regName + " , " + node.right->regName);
            code.emit("br i1 " + checkRegister + ", label " + labelTrue + " , label " + labelfalse);
            code.emitLabel(labelTrue);
            code.emit(trueRegister + " = add i32 0,1");
            code.emit("br label " + assignPhi);
            code.emitLabel(labelfalse);
            code.emit(falseRegister + " = add i32 0,0");
            code.emit("br label " + assignPhi);
            code.emitLabel(assignPhi);
            code.emit(node.regName + " = phi i32 [" + trueRegister + ", " + labelTrue + "], [" + falseRegister + ", " + labelfalse + "]");
            return;
        }
        code.emit(node.regName + " =icmp " + op + " i32 " + node.left->regName + " , " + node.right->regName);
        code.emit("br i1 " + node.regName + ", label " + node.trueLabel + " , label " + node.falseLabel);
    }

    void CodeGeneratorVisitor::visit(ast::Not& node)
    {
        node.exp->falseLabel = node.trueLabel;
        node.exp->trueLabel = node.falseLabel;
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
        if (checkingBoolForCall)
        {
            node.regName = code.freshVar();
            //special case, need to return normal bool
            std::string checkRegister = code.freshVar();
            std::string assignPhi = code.freshLabel();
            std::string trueRegister = code.freshVar();
            std::string falseRegister = code.freshVar();
            std::string labelTrue = code.freshLabel();
            std::string labelfalse = code.freshLabel();
            code.emit(checkRegister + " =icmp eq i32 0 , " + node.exp->regName);
            code.emit("br i1 " + checkRegister + ", label " + labelTrue + " , label " + labelfalse);
            code.emitLabel(labelTrue);
            code.emit(trueRegister + " = add i32 0,1");
            code.emit("br label " + assignPhi);
            code.emitLabel(labelfalse);
            code.emit(falseRegister + " = add i32 0,0");
            code.emit("br label " + assignPhi);
            code.emitLabel(assignPhi);
            code.emit(node.regName + " = phi i32 [" + trueRegister + ", " + labelTrue + "], [" + falseRegister + ", " + labelfalse + "]");
        }
    }

    void CodeGeneratorVisitor::visit(ast::And& node)
    {
        if (checkingBoolForCall)
        {
            node.regName = code.freshVar();
            //special case, need to return normal bool
            std::string midLabel = code.freshLabel();
            std::string leftEval = code.freshVar();
            std::string rightEval = code.freshVar();
            std::string assignPhi = code.freshLabel();
            std::string trueRegister = code.freshVar();
            std::string falseRegister = code.freshVar();
            std::string labelTrue = code.freshLabel();
            std::string labelfalse = code.freshLabel();
            //left eval
            node.left->accept(*this);
            code.emit(leftEval + " =icmp eq i32 1 , " + node.left->regName);
            code.emit("br i1 " + leftEval + ", label " + midLabel + " , label " + labelfalse);
            code.emitLabel(midLabel);
            node.right->accept(*this);
            code.emit(rightEval + " =icmp eq i32 1 , " + node.right->regName);
            code.emit("br i1 " + rightEval + ", label " + labelTrue + " , label " + labelfalse);
            code.emitLabel(labelTrue);
            code.emit(trueRegister + " = add i32 0,1");
            code.emit("br label " + assignPhi);
            code.emitLabel(labelfalse);
            code.emit(falseRegister + " = add i32 0,0");
            code.emit("br label " + assignPhi);
            code.emitLabel(assignPhi);
            code.emit(node.regName + " = phi i32 [" + trueRegister + ", " + labelTrue + "], [" + falseRegister + ", " + labelfalse + "]");
        }
        else
        {
            node.left->trueLabel = code.freshLabel();
            node.left->falseLabel = node.falseLabel;
            node.left->accept(*this);
            code.emitLabel(node.left->trueLabel);
            node.right->trueLabel = node.trueLabel;
            node.right->falseLabel = node.falseLabel;
            node.right->accept(*this);
        }
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

    void CodeGeneratorVisitor::visit(ast::Or& node)
    {
        if (checkingBoolForCall)
        {
            node.regName = code.freshVar();
            //special case, need to return normal bool
            std::string midLabel = code.freshLabel();
            std::string leftEval = code.freshVar();
            std::string rightEval = code.freshVar();
            std::string assignPhi = code.freshLabel();
            std::string trueRegister = code.freshVar();
            std::string falseRegister = code.freshVar();
            std::string labelTrue = code.freshLabel();
            std::string labelfalse = code.freshLabel();
            //left eval
            node.left->accept(*this);
            code.emit(leftEval + " =icmp eq i32 1 , " + node.left->regName);
            code.emit("br i1 " + leftEval + ", label " + labelTrue + " , label " + midLabel);
            code.emitLabel(midLabel);
            node.right->accept(*this);
            code.emit(rightEval + " =icmp eq i32 1 , " + node.right->regName);
            code.emit("br i1 " + rightEval + ", label " + labelTrue + " , label " + labelfalse);
            code.emitLabel(labelTrue);
            code.emit(trueRegister + " = add i32 0,1");
            code.emit("br label " + assignPhi);
            code.emitLabel(labelfalse);
            code.emit(falseRegister + " = add i32 0,0");
            code.emit("br label " + assignPhi);
            code.emitLabel(assignPhi);
            code.emit(node.regName + " = phi i32 [" + trueRegister + ", " + labelTrue + "], [" + falseRegister + ", " + labelfalse + "]");
        }
        else
        {
            node.left->trueLabel = node.trueLabel;
            node.left->falseLabel = code.freshLabel();
            node.left->accept(*this);
            code.emitLabel(node.left->falseLabel);
            node.right->trueLabel = node.trueLabel;
            node.right->falseLabel = node.falseLabel;
            node.right->accept(*this);
        }
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

    void CodeGeneratorVisitor::visit(ast::Type& node)
    {

    }

    void CodeGeneratorVisitor::visit(ast::Cast& node)
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
            node.regName = code.freshVar();
            code.emit(node.regName + " = add i32 0, " + node.exp->regName);
            return;
        }
        if (toString(node.target_type->type) == "byte" && (node.exp->expType == "int" || node.exp->expType == "byte"))
        {
            node.expType = "byte";
            node.regName = code.freshVar();
            code.emit(node.regName + " = and i32 255, " + node.exp->regName);
            return;
        }
        output::errorMismatch(node.line);
    }

    void CodeGeneratorVisitor::visit(ast::ExpList& node)
    {
        for (std::shared_ptr<ast::Exp> exp : node.exps)
        {
            exp->accept(*this);
        }
    }

    void CodeGeneratorVisitor::visit(ast::Call& node)
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
        checkingBoolForCall = true;
        node.args->accept(*this); // make sure that the elements give are OK
        checkingBoolForCall = false;
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
        //now lets create the code
        node.regName = code.freshVar();
        std::string argumentsForCall = "";
        std::vector<std::shared_ptr<ast::Exp>> list = node.args->exps;
        int argumentsNum = list.size();
        for (int i=0; i<argumentsNum;i++)
        {
            std::shared_ptr<ast::Exp>param = list[i];
            if (param->expType == "string")
            {
                argumentsForCall += "i8*";
            }
            else 
            {
                argumentsForCall += "i32";
            }
            argumentsForCall += " ";
            argumentsForCall += param->regName;
            if (i < argumentsNum - 1)
                argumentsForCall += ", ";
        }
        if (node.expType == "bool")
        {
            //special care, maybe called in a boolean
            code.emit(node.regName + " = call i32 @" + node.func_id->value + "(" + argumentsForCall + ")");
            if (node.falseLabel != "")
            {
                //we actually need it
                std::string compare = code.freshVar();
                code.emit(compare + " = icmp eq i32 1, " + node.regName);
                code.emit("br i1 " + compare + " ,label " + node.trueLabel + " ,label " + node.falseLabel);
            }
        }
        else
        {
            if (node.expType == "void")
            {
                code.emit("call void @" + node.func_id->value + "(" + argumentsForCall + ")");
            }
            else
            {
                //int and byte
                code.emit(node.regName +" = call i32 @" + node.func_id->value + "(" + argumentsForCall + ")");
            }
        }
    }

    void CodeGeneratorVisitor::visit(ast::Statements& node)
    {
        bool is_loop = stack.getCurrentScope()->isScopeOfLoop;
        if (node.hasParen)
        {
            stack.push(is_loop, false, stack.getCurrentScope()->typeOfReturn);
        }
        std::vector<std::shared_ptr<ast::Statement>> elems = node.statements;
        std::string thisLabel = code.freshLabel();
        code.emit("br label " + thisLabel);
        std::string nextLabel;
        for (std::shared_ptr<ast::Statement> st : elems)
        {
            st->innerLoopStart = node.innerLoopStart;
            st->innerLoopNext = node.innerLoopNext;
            nextLabel = code.freshLabel();
            st->thisLabel = thisLabel;
            st->nextLabel = nextLabel;
            code.emitLabel(thisLabel);
            st->accept(*this);
            code.emit("br label " + nextLabel);
            thisLabel = nextLabel;
        }
        code.emitLabel(thisLabel);
        if (node.hasParen)
        {
            stack.pop();
        }
    }

    void CodeGeneratorVisitor::visit(ast::Break& node)
    {
        if (!stack.getCurrentScope()->isScopeOfLoop)
        {
            output::errorUnexpectedBreak(node.line);
        }
        code.emit("br label " + node.innerLoopNext);
    }

    void CodeGeneratorVisitor::visit(ast::Continue& node)
    {
        if (!stack.getCurrentScope()->isScopeOfLoop)
        {
            output::errorUnexpectedContinue(node.line);
        }
        code.emit("br label " + node.innerLoopStart);
    }

    void CodeGeneratorVisitor::visit(ast::Return& node)
    {
        if (node.exp == nullptr) //return void
        {
            if (stack.getCurrentScope()->typeOfReturn != "void")
            {
                output::errorMismatch(node.line);
            }
            code.emit("ret void");
            return;
        }
        //bool helper
        std::string labeltrue = code.freshLabel();
        std::string labelfalse = code.freshLabel();
        node.exp->trueLabel = labeltrue;
        node.exp->falseLabel = labelfalse;
        //
        node.exp->accept(*this);
        if (isFunction(node.exp))
        {
            output::errorDefAsFunc(node.line, std::dynamic_pointer_cast<ast::ID>(node.exp)->value);
        }
        if (node.exp->expType != stack.getCurrentScope()->typeOfReturn)
        {
            if (node.exp->expType == "byte" && stack.getCurrentScope()->typeOfReturn == "int")
            {
                code.emit("ret i32 " + node.exp->regName);
                return;
            }
            output::errorMismatch(node.line);
        }
        if (node.exp->expType == "bool")
        {
            //a bit more complex
            std::string returnRegister = code.freshVar();
            std::string assignPhi = code.freshLabel();
            std::string trueRegister = code.freshVar();
            std::string falseRegister = code.freshVar();
            code.emitLabel(labeltrue);
            code.emit(trueRegister + " = add i32 0,1");
            code.emit("br label " + assignPhi);
            code.emitLabel(labelfalse);
            code.emit(falseRegister + " = add i32 0,0");
            code.emit("br label " + assignPhi);
            code.emitLabel(assignPhi);
            code.emit(returnRegister + " = phi i32 [" + trueRegister + ", " + labeltrue + "], [" + falseRegister + ", " + labelfalse + "]");
            code.emit("ret i32 " + returnRegister);
            return;
        }
        if (node.exp->expType == "string")
        {
            code.emit("ret i8* " + node.exp->regName);
        }
        else
        {
            code.emit("ret i32 " + node.exp->regName);
        }
    }

    void CodeGeneratorVisitor::visit(ast::If& node)
    {
        stack.push(stack.getCurrentScope()->isScopeOfLoop, false, stack.getCurrentScope()->typeOfReturn);
        node.condition->trueLabel = code.freshLabel();
        if (node.otherwise != nullptr)
        {
            node.condition->falseLabel = code.freshLabel();
        }
        else
        {
            node.condition->falseLabel = node.nextLabel;
        }
        node.condition->accept(*this);

        if (node.condition->expType != "bool")
        {
            output::errorMismatch(node.condition->line);
        }
        code.emitLabel(node.condition->trueLabel);
        node.then->innerLoopNext = node.innerLoopNext;
        node.then->innerLoopStart = node.innerLoopStart;
        node.then->nextLabel = node.nextLabel;
        node.then->accept(*this);
        code.emit("br label " + node.nextLabel);
        stack.pop();

        if (node.otherwise != nullptr) // there is an else
        {
            stack.push(stack.getCurrentScope()->isScopeOfLoop, false, stack.getCurrentScope()->typeOfReturn);
            code.emitLabel(node.condition->falseLabel);
            node.otherwise->innerLoopNext = node.innerLoopNext;
            node.otherwise->innerLoopStart = node.innerLoopStart;
            node.otherwise->nextLabel = node.nextLabel;
            node.otherwise->accept(*this);
            code.emit("br label " + node.nextLabel);
            stack.pop();
        }
    }

    void CodeGeneratorVisitor::visit(ast::While& node)
    {
        stack.push(true, false, stack.getCurrentScope()->typeOfReturn);
        node.condition->trueLabel = code.freshLabel();
        node.condition->falseLabel = node.nextLabel;
        node.condition->accept(*this);

        if (node.condition->expType != "bool")
        {
            output::errorMismatch(node.condition->line);
        }
        code.emitLabel(node.condition->trueLabel);
        node.body->innerLoopStart = node.thisLabel;
        node.body->innerLoopNext = node.nextLabel;
        node.body->accept(*this);
        code.emit("br label " + node.thisLabel);
        stack.pop();
    }

    void CodeGeneratorVisitor::visit(ast::VarDecl& node)
    {
        node.type->accept(*this);
        if (stack.isSymbolExistsInAnyScope(node.id->value))
        {
            output::errorDef(node.line, node.id->value);
        }
        std::string labeltrue = code.freshLabel();
        std::string labelfalse = code.freshLabel();
        if (node.init_exp != nullptr) //initial
        {
            node.init_exp->trueLabel = labeltrue;
            node.init_exp->falseLabel = labelfalse;
            node.init_exp->accept(*this);
        }
        stack.addSymbol(node.id->value, toString(node.type->type), false);
        std::shared_ptr<SingleSymbol> var = stack.getSymbol(node.id->value);
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
        //create code
        std::string assignRegister = code.freshVar();
        if (node.init_exp == nullptr)
        {
            if (node.id->expType == "int" || node.id->expType == "byte")
            {
                code.emit(assignRegister + " = add i32 0, 0");
            }
            else
            {
                code.emit(assignRegister + " = add i32 0, 0");
            }
            storeInStack(assignRegister, stack.getCurrentScope()->baseRegister, var->symbolOffset);
        }
        else
        {
            if (node.init_exp->expType != "bool")
            {
                code.emit(assignRegister + " = add i32 0, " + node.init_exp->regName);
                storeInStack(assignRegister, stack.getCurrentScope()->baseRegister, var->symbolOffset);
            }
            else
            {
                //a bit more complex
                std::string assignPhi = code.freshLabel();
                std::string trueRegister = code.freshVar();
                std::string falseRegister = code.freshVar();
                code.emitLabel(labeltrue);
                code.emit(trueRegister + " = add i32 0,1");
                code.emit("br label " + assignPhi);
                code.emitLabel(labelfalse);
                code.emit(falseRegister + " = add i32 0,0");
                code.emit("br label " + assignPhi);
                code.emitLabel(assignPhi);
                code.emit(assignRegister + " = phi i32 ["+trueRegister+", "+labeltrue+"], ["+falseRegister+", "+labelfalse+"]");
                storeInStack(assignRegister, stack.getCurrentScope()->baseRegister, var->symbolOffset);
                code.emit("br label " + node.nextLabel);
            }
        }
    }

    void CodeGeneratorVisitor::visit(ast::Assign& node)
    {
        std::string labeltrue = code.freshLabel();
        std::string labelfalse = code.freshLabel();
        node.exp->trueLabel = labeltrue;
        node.exp->falseLabel = labelfalse;
        
        //node.id->accept(*this);
        //a bit pronlomatic, let's change it
        std::shared_ptr<SingleSymbol>symbol = stack.getSymbol(node.id->value);
        if (symbol != nullptr)
        {
            node.id->expType = symbol->type;
        }
        else
        {
            output::errorUndef(node.id->line, node.id->value);
        }
        //
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
        std::string assignRegister = code.freshVar();
        if (node.exp->expType != "bool")
        {
            code.emit(assignRegister + " = add i32 0, " + node.exp->regName);
            storeInStack(assignRegister, stack.getCurrentScope()->baseRegister, var->symbolOffset);
        }
        else
        {
            //a bit more complex
            std::string assignPhi = code.freshLabel();
            std::string trueRegister = code.freshVar();
            std::string falseRegister = code.freshVar();
            code.emitLabel(labeltrue);
            code.emit(trueRegister + " = add i32 0,1");
            code.emit("br label " + assignPhi);
            code.emitLabel(labelfalse);
            code.emit(falseRegister + " = add i32 0,0");
            code.emit("br label " + assignPhi);
            code.emitLabel(assignPhi);
            code.emit(assignRegister + " = phi i32 [" + trueRegister + ", " + labeltrue + "], [" + falseRegister + ", " + labelfalse + "]");
            storeInStack(assignRegister, stack.getCurrentScope()->baseRegister, var->symbolOffset);
            code.emit("br label " + node.nextLabel);
        }
    }

    void CodeGeneratorVisitor::visit(ast::Formal& node)
    {
        node.type->accept(*this);
        stack.addSymbol(node.id->value, toString(node.type->type), false);
    }

    void CodeGeneratorVisitor::visit(ast::Formals& node)
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
            counter--;
        }
    }

    void CodeGeneratorVisitor::visit(ast::FuncDecl& node)
    {
        //here, the defenition already was made and types of formals were checked
        //we only need to enter the scope
        stack.push(false, true, toString(node.return_type->type));
        //start function
        //allocate stack
        std::string stackRegister = code.freshVar();
        code.emit(stackRegister + " = alloca i32,i32 50");
        stack.getCurrentScope()->baseRegister = stackRegister;
        //
        node.formals->accept(*this);
        node.body->accept(*this);
        //end function
        if (node.return_type->type == ast::BuiltInType::VOID) 
        {
            code.emit("ret void");
        }
        else 
        {
            code.emit("ret i32 0");
        }
        code.emit("}");

        stack.pop();
    }

    void CodeGeneratorVisitor::visit(ast::Funcs& node)
    {
        std::vector<std::shared_ptr<ast::FuncDecl>>funcs = node.funcs;
        //enter all defenitions to global scope
        //need to make sure
        // 1) main exists
        // 2) no two functions with the same name
        // 3) no two variables from the same function with the same name
        bool mainExists = false;
        bool mainHere = false;
        for (std::shared_ptr<ast::FuncDecl>function : funcs)
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
            if (mainHere)
            {
                if (formalNames.size() != 0 || toString(function->return_type->type) != "void")
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
        //declare the functions
        for (std::shared_ptr<ast::FuncDecl>function : funcs)
        {
            std::string arguments = "";
            int argSize = function->formals->formals.size();
            for (int i = 0; i < argSize; i++)
            {
                std::shared_ptr<ast::Formal> paramDecl = (function->formals->formals)[i];
                if (paramDecl->type->type == ast::BuiltInType::STRING)
                {
                    arguments += "i8*";
                }
                else
                {
                    arguments += "i32";
                }
                if (i < argSize - 1)
                {
                    arguments += ", ";
                }
            }
            std::string returnType = "i32";
            if (function->return_type->type == ast::BuiltInType::STRING)
            {
                returnType = "i8*";
            }
            else
            {
                if (function->return_type->type == ast::BuiltInType::VOID)
                {
                    returnType = "void";
                }
            }
            code.emit("define " + returnType + " @" + function->id->value + "(" + arguments + "){");
            //actually make the actions
            function->accept(*this);
        }
    }

    bool CodeGeneratorVisitor::isFunction(std::shared_ptr<ast::Exp> node)
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

    //loads a variable from the stack
    std::string CodeGeneratorVisitor::loadFromStack(std::string stackBase, int offset)
    {
        std::string registerLoaded = code.freshVar();
        std::string ptr = code.freshVar();
        code.emit(ptr + " = getelementptr i32, i32* " + stackBase + ", i32 " + std::to_string(offset));
        code.emit(registerLoaded + " = load i32, i32* " + ptr);
        return registerLoaded;
    }

    //stores a variable in the stack
    void CodeGeneratorVisitor::storeInStack(std::string registerName, std::string stackBase, int offset)
    {
        std::string ptr = code.freshVar();
        code.emit(ptr + " = getelementptr i32, i32* " + stackBase + ", i32 " + std::to_string(offset));
        code.emit("store i32 " + registerName + ", i32* " + ptr);
    }

    void CodeGeneratorVisitor::printEnd()
    {
        std::cout << code;
    }
}
