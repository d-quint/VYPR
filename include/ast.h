#ifndef VYPR_AST_H
#define VYPR_AST_H

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "token.h"
#include <iostream>

namespace vypr {

// Forward declarations
class Expression;
class Statement;

// Base AST node class
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(std::ostream& out, int indent = 0) const = 0;
};

// Expression types
using ExpressionPtr = std::shared_ptr<Expression>;
using StatementPtr = std::shared_ptr<Statement>;

// Literal value types
using LiteralValue = std::variant<int, double, bool, std::string>;

// Base Expression class
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Literal expression (numbers, strings, booleans)
class LiteralExpression : public Expression {
public:
    LiteralValue value;
    
    explicit LiteralExpression(LiteralValue value) : value(std::move(value)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Literal: ";
        std::visit([&out](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
                out << "\"" << arg << "\"";
            else if constexpr (std::is_same_v<T, bool>)
                out << (arg ? "true" : "false");
            else
                out << arg;
        }, value);
        out << "\n";
    }
};

// Variable reference expression
class VariableExpression : public Expression {
public:
    std::string name;
    
    explicit VariableExpression(std::string name) : name(std::move(name)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Variable: " << name << "\n";
    }
};

// Binary operation expression
class BinaryExpression : public Expression {
public:
    ExpressionPtr left;
    TokenType op;
    ExpressionPtr right;
    
    BinaryExpression(ExpressionPtr left, TokenType op, ExpressionPtr right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "BinaryOp: " << tokenTypeToString(op) << "\n";
        left->print(out, indent + 2);
        right->print(out, indent + 2);
    }
};

// Unary operation expression
class UnaryExpression : public Expression {
public:
    TokenType op;
    ExpressionPtr right;
    
    UnaryExpression(TokenType op, ExpressionPtr right)
        : op(op), right(std::move(right)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "UnaryOp: " << tokenTypeToString(op) << "\n";
        right->print(out, indent + 2);
    }
};

// Array access expression: array[index]
class ArrayAccessExpression : public Expression {
public:
    ExpressionPtr array;
    ExpressionPtr index;
    
    ArrayAccessExpression(ExpressionPtr array, ExpressionPtr index)
        : array(std::move(array)), index(std::move(index)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "ArrayAccess:\n";
        out << std::string(indent + 2, ' ') << "Array:\n";
        array->print(out, indent + 4);
        out << std::string(indent + 2, ' ') << "Index:\n";
        index->print(out, indent + 4);
    }
};

// Method call expression: obj.method()
class MemberAccessExpression : public Expression {
public:
    ExpressionPtr object;
    std::string member;
    
    MemberAccessExpression(ExpressionPtr object, std::string member)
        : object(std::move(object)), member(std::move(member)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "MemberAccess: " << member << "\n";
        out << std::string(indent + 2, ' ') << "Object:\n";
        object->print(out, indent + 4);
    }
};

// Function call expression
class CallExpression : public Expression {
public:
    std::string callee;
    std::vector<ExpressionPtr> arguments;
    
    CallExpression(std::string callee, std::vector<ExpressionPtr> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Call: " << callee << "\n";
        out << std::string(indent + 2, ' ') << "Arguments:\n";
        for (const auto& arg : arguments) {
            arg->print(out, indent + 4);
        }
    }
};

// Array literal expression
class ArrayExpression : public Expression {
public:
    std::vector<ExpressionPtr> elements;
    
    explicit ArrayExpression(std::vector<ExpressionPtr> elements)
        : elements(std::move(elements)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "ArrayLiteral:\n";
        out << std::string(indent + 2, ' ') << "Elements:\n";
        for (const auto& elem : elements) {
            elem->print(out, indent + 4);
        }
    }
};

// Base Statement class
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Expression statement
class ExpressionStatement : public Statement {
public:
    ExpressionPtr expression;
    
    explicit ExpressionStatement(ExpressionPtr expression)
        : expression(std::move(expression)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "ExpressionStatement:\n";
        expression->print(out, indent + 2);
    }
};

// Variable declaration statement
class VarDeclarationStatement : public Statement {
public:
    std::string name;
    ExpressionPtr initializer;
    
    VarDeclarationStatement(std::string name, ExpressionPtr initializer)
        : name(std::move(name)), initializer(std::move(initializer)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "VarDecl: " << name;
        if (initializer) {
            out << " =\n";
            initializer->print(out, indent + 2);
        } else {
            out << "\n";
        }
    }
};

// Block statement
class BlockStatement : public Statement {
public:
    std::vector<StatementPtr> statements;
    
    explicit BlockStatement(std::vector<StatementPtr> statements)
        : statements(std::move(statements)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Block:\n";
        for (const auto& stmt : statements) {
            stmt->print(out, indent + 2);
        }
    }
};

// If statement
class IfStatement : public Statement {
public:
    ExpressionPtr condition;
    StatementPtr then_branch;
    StatementPtr else_branch;
    
    IfStatement(ExpressionPtr condition, StatementPtr then_branch, StatementPtr else_branch)
        : condition(std::move(condition)), then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "If:\n";
        out << std::string(indent + 2, ' ') << "Condition:\n";
        condition->print(out, indent + 4);
        out << std::string(indent + 2, ' ') << "Then:\n";
        then_branch->print(out, indent + 4);
        if (else_branch) {
            out << std::string(indent + 2, ' ') << "Else:\n";
            else_branch->print(out, indent + 4);
        }
    }
};

// While statement
class WhileStatement : public Statement {
public:
    ExpressionPtr condition;
    StatementPtr body;
    
    WhileStatement(ExpressionPtr condition, StatementPtr body)
        : condition(std::move(condition)), body(std::move(body)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "While:\n";
        out << std::string(indent + 2, ' ') << "Condition:\n";
        condition->print(out, indent + 4);
        out << std::string(indent + 2, ' ') << "Body:\n";
        body->print(out, indent + 4);
    }
};

// Loop-in statement (for-each loop)
class LoopInStatement : public Statement {
public:
    std::string variable;
    ExpressionPtr iterable;
    StatementPtr body;
    
    LoopInStatement(std::string variable, ExpressionPtr iterable, StatementPtr body)
        : variable(std::move(variable)), iterable(std::move(iterable)), body(std::move(body)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "LoopIn: " << variable << "\n";
        out << std::string(indent + 2, ' ') << "Iterable:\n";
        iterable->print(out, indent + 4);
        out << std::string(indent + 2, ' ') << "Body:\n";
        body->print(out, indent + 4);
    }
};

// Loop-times statement (repeat N times)
class LoopTimesStatement : public Statement {
public:
    ExpressionPtr count;
    StatementPtr body;
    
    LoopTimesStatement(ExpressionPtr count, StatementPtr body)
        : count(std::move(count)), body(std::move(body)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "LoopTimes:\n";
        out << std::string(indent + 2, ' ') << "Count:\n";
        count->print(out, indent + 4);
        out << std::string(indent + 2, ' ') << "Body:\n";
        body->print(out, indent + 4);
    }
};

// Return statement
class ReturnStatement : public Statement {
public:
    ExpressionPtr value;
    
    explicit ReturnStatement(ExpressionPtr value)
        : value(std::move(value)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Return:";
        if (value) {
            out << "\n";
            value->print(out, indent + 2);
        } else {
            out << " (void)\n";
        }
    }
};

// Print statement
class PrintStatement : public Statement {
public:
    ExpressionPtr expression;
    
    explicit PrintStatement(ExpressionPtr expression)
        : expression(std::move(expression)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Print:\n";
        expression->print(out, indent + 2);
    }
};

// Input statement
class InputStatement : public Statement {
public:
    std::string variable;
    
    explicit InputStatement(std::string variable)
        : variable(std::move(variable)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Input: " << variable << "\n";
    }
};

// Function declaration statement
class FunctionDeclaration : public Statement {
public:
    std::string name;
    std::vector<std::string> parameters;
    StatementPtr body;
    
    FunctionDeclaration(std::string name, std::vector<std::string> parameters, StatementPtr body)
        : name(std::move(name)), parameters(std::move(parameters)), body(std::move(body)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Function: " << name << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            out << parameters[i] << (i < parameters.size() - 1 ? ", " : "");
        }
        out << ")\n";
        body->print(out, indent + 2);
    }
};

// Program is the root AST node
class Program : public ASTNode {
public:
    std::vector<StatementPtr> statements;
    
    explicit Program(std::vector<StatementPtr> statements)
        : statements(std::move(statements)) {}

    void print(std::ostream& out, int indent = 0) const override {
        out << std::string(indent, ' ') << "Program:\n";
        for (const auto& stmt : statements) {
            stmt->print(out, indent + 2);
        }
    }
};

} // namespace vypr

#endif // VYPR_AST_H 