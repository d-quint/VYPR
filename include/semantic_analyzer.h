#ifndef VYPR_SEMANTIC_ANALYZER_H
#define VYPR_SEMANTIC_ANALYZER_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "ast.h"
#include "exceptions.h"

namespace vypr {

// Symbol represents a variable or function in the symbol table
struct Symbol {
    enum class Type {
        VARIABLE,
        FUNCTION
    };
    
    Type type;
    bool initialized;
    int paramCount;  // Only for functions
    
    explicit Symbol(Type type, bool initialized = true, int paramCount = 0)
        : type(type), initialized(initialized), paramCount(paramCount) {}
};

class Scope {
public:
    std::unordered_map<std::string, Symbol> symbols;
    Scope* parent;
    
    explicit Scope(Scope* parent = nullptr) : parent(parent) {}
    
    bool define(const std::string& name, const Symbol& symbol);
    bool isDefined(const std::string& name) const;
    Symbol* resolve(const std::string& name);
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    void analyze(const std::shared_ptr<Program>& program);
    void printSymbolTable() const;

private:
    Scope* current_scope;
    bool in_function;
    
    void enterScope();
    void exitScope();
    
    // Visitor methods for each AST node type
    void visit(const std::shared_ptr<Program>& node);
    void visit(const StatementPtr& stmt);
    void visit(const ExpressionPtr& expr);
    
    void visitVarDeclaration(const std::shared_ptr<VarDeclarationStatement>& stmt);
    void visitFunctionDeclaration(const std::shared_ptr<FunctionDeclaration>& stmt);
    void visitExpressionStatement(const std::shared_ptr<ExpressionStatement>& stmt);
    void visitIfStatement(const std::shared_ptr<IfStatement>& stmt);
    void visitWhileStatement(const std::shared_ptr<WhileStatement>& stmt);
    void visitReturnStatement(const std::shared_ptr<ReturnStatement>& stmt);
    void visitBlockStatement(const std::shared_ptr<BlockStatement>& stmt);
    void visitPrintStatement(const std::shared_ptr<PrintStatement>& stmt);
    void visitInputStatement(const std::shared_ptr<InputStatement>& stmt);
    void visitLoopInStatement(const std::shared_ptr<LoopInStatement>& stmt);
    void visitLoopTimesStatement(const std::shared_ptr<LoopTimesStatement>& stmt);
    
    void visitBinaryExpression(const std::shared_ptr<BinaryExpression>& expr);
    void visitUnaryExpression(const std::shared_ptr<UnaryExpression>& expr);
    void visitLiteralExpression(const std::shared_ptr<LiteralExpression>& expr);
    void visitVariableExpression(const std::shared_ptr<VariableExpression>& expr);
    void visitCallExpression(const std::shared_ptr<CallExpression>& expr);
    void visitArrayExpression(const std::shared_ptr<ArrayExpression>& expr);
    void visitArrayAccessExpression(const std::shared_ptr<ArrayAccessExpression>& expr);
    void visitMemberAccessExpression(const std::shared_ptr<MemberAccessExpression>& expr);
};

} // namespace vypr

#endif // VYPR_SEMANTIC_ANALYZER_H 