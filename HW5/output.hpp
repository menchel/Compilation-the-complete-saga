#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <vector>
#include <string>
#include <sstream>
#include "visitor.hpp"
#include "nodes.hpp"
#include "symbolTable.hpp"

namespace output {
    /* Error handling functions */

    void errorLex(int lineno);

    void errorSyn(int lineno);

    void errorUndef(int lineno, const std::string &id);

    void errorDefAsFunc(int lineno, const std::string &id);

    void errorUndefFunc(int lineno, const std::string &id);

    void errorDefAsVar(int lineno, const std::string &id);

    void errorDef(int lineno, const std::string &id);

    void errorPrototypeMismatch(int lineno, const std::string &id, std::vector<std::string> &paramTypes);

    void errorMismatch(int lineno);

    void errorUnexpectedBreak(int lineno);

    void errorUnexpectedContinue(int lineno);

    void errorMainMissing();

    void errorByteTooLarge(int lineno, int value);

    /* CodeBuffer class
     * This class is used to store the generated code.
     * It provides a simple interface to emit code and manage labels and variables.
     */
    class CodeBuffer {
    private:
        std::stringstream globalsBuffer;
        std::stringstream buffer;
        int labelCount;
        int varCount;
        int stringCount;

        friend std::ostream &operator<<(std::ostream &os, const CodeBuffer &buffer);

    public:
        CodeBuffer();

        // Returns a string that represents a label not used before
        // Usage examples:
        //      emitLabel(freshLabel());
        //      buffer << "br label " << freshLabel() << std::endl;
        std::string freshLabel();

        // Returns a string that represents a variable not used before
        // Usage examples:
        //      std::string var = freshVar();
        //      buffer << var << " = icmp eq i32 0, 0" << std::endl;
        std::string freshVar();

        // Emits a label into the buffer
        void emitLabel(const std::string &label);

        // Emits a constant string into the globals section of the code.
        // Returns the name of the constant. For the string of the length n (not including null character), the type is [n+1 x i8]
        // Usage examples:
        //      std::string str = emitString("Hello, World!");
        //      buffer << "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* " << str << ", i32 0, i32 0))" << std::endl;
        std::string emitString(const std::string &str);

        // Emits a string into the buffer
        void emit(const std::string &str);

        // Template overload for general types
        template<typename T>
        CodeBuffer &operator<<(const T &value) {
            buffer << value;
            return *this;
        }

        // Overload for manipulators (like std::endl)
        CodeBuffer &operator<<(std::ostream &(*manip)(std::ostream &));
    };

    std::ostream &operator<<(std::ostream &os, const CodeBuffer &buffer);

    class CodeGeneratorVisitor : public Visitor
    {
        CodeBuffer code;
        StackSymbolTables stack;
        bool checkingBoolForCall;

    public:
        CodeGeneratorVisitor();

        void visit(ast::Num& node) override;

        void visit(ast::NumB& node) override;

        void visit(ast::String& node) override;

        void visit(ast::Bool& node) override;

        void visit(ast::ID& node) override;

        void visit(ast::BinOp& node) override;

        void visit(ast::RelOp& node) override;

        void visit(ast::Not& node) override;

        void visit(ast::And& node) override;

        void visit(ast::Or& node) override;

        void visit(ast::Type& node) override;

        void visit(ast::Cast& node) override;

        void visit(ast::ExpList& node) override;

        void visit(ast::Call& node) override;

        void visit(ast::Statements& node) override;

        void visit(ast::Break& node) override;

        void visit(ast::Continue& node) override;

        void visit(ast::Return& node) override;

        void visit(ast::If& node) override;

        void visit(ast::While& node) override;

        void visit(ast::VarDecl& node) override;

        void visit(ast::Assign& node) override;

        void visit(ast::Formal& node) override;

        void visit(ast::Formals& node) override;

        void visit(ast::FuncDecl& node) override;

        void visit(ast::Funcs& node) override;

        bool isFunction(std::shared_ptr<ast::Exp> node);

        std::string loadFromStack(std::string stackBase, int offset);

        void storeInStack(std::string registerName, std::string stackBase, int offset);

        void printEnd();
    };
}

#endif //OUTPUT_HPP
