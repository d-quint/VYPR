#include "semantic_analyzer.h"
#include <sstream>
#include <iostream>

namespace vypr {

bool Scope::define(const std::string& name, const Symbol& symbol) {
    // Check if symbol already exists in this scope
    if (symbols.find(name) != symbols.end()) {
        return false;
    }
    
    symbols.insert({name, symbol});
    return true;
}

bool Scope::isDefined(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

Symbol* Scope::resolve(const std::string& name) {
    // Check in current scope
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    
    // Check in parent scope
    if (parent != nullptr) {
        return parent->resolve(name);
    }
    
    return nullptr;
}

SemanticAnalyzer::SemanticAnalyzer() : current_scope(nullptr), in_function(false) {
    // Start with global scope
    current_scope = new Scope();
}

void SemanticAnalyzer::analyze(const std::shared_ptr<Program>& program) {
    try {
        visit(program);
    } catch (const SemanticError& e) {
        // Clean up scopes
        while (current_scope != nullptr) {
            Scope* parent = current_scope->parent;
            delete current_scope;
            current_scope = parent;
        }
        
        // Re-throw with more context
        throw;
    }
    
    // Clean up scopes
    while (current_scope != nullptr) {
        Scope* parent = current_scope->parent;
        delete current_scope;
        current_scope = parent;
    }
}

void SemanticAnalyzer::enterScope() {
    current_scope = new Scope(current_scope);
}

void SemanticAnalyzer::exitScope() {
    if (current_scope == nullptr) {
        throw SemanticError("Internal error: Trying to exit non-existent scope");
    }
    
    Scope* parent = current_scope->parent;
    delete current_scope;
    current_scope = parent;
}

void SemanticAnalyzer::visit(const std::shared_ptr<Program>& node) {
    // Visit all statements in the program
    for (const auto& stmt : node->statements) {
        visit(stmt);
    }
}

void SemanticAnalyzer::visit(const StatementPtr& stmt) {
    if (auto varDecl = std::dynamic_pointer_cast<VarDeclarationStatement>(stmt)) {
        visitVarDeclaration(varDecl);
    } else if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclaration>(stmt)) {
        visitFunctionDeclaration(funcDecl);
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        visitExpressionStatement(exprStmt);
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        visitIfStatement(ifStmt);
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
        visitWhileStatement(whileStmt);
    } else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        visitReturnStatement(returnStmt);
    } else if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        visitBlockStatement(blockStmt);
    } else if (auto printStmt = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        visitPrintStatement(printStmt);
    } else if (auto inputStmt = std::dynamic_pointer_cast<InputStatement>(stmt)) {
        visitInputStatement(inputStmt);
    } else if (auto loopInStmt = std::dynamic_pointer_cast<LoopInStatement>(stmt)) {
        visitLoopInStatement(loopInStmt);
    } else if (auto loopTimesStmt = std::dynamic_pointer_cast<LoopTimesStatement>(stmt)) {
        visitLoopTimesStatement(loopTimesStmt);
    } else {
        throw SemanticError("Unknown statement type");
    }
}

void SemanticAnalyzer::visit(const ExpressionPtr& expr) {
    if (auto binary = std::dynamic_pointer_cast<BinaryExpression>(expr)) {
        visitBinaryExpression(binary);
    } else if (auto unary = std::dynamic_pointer_cast<UnaryExpression>(expr)) {
        visitUnaryExpression(unary);
    } else if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(expr)) {
        visitLiteralExpression(literal);
    } else if (auto variable = std::dynamic_pointer_cast<VariableExpression>(expr)) {
        visitVariableExpression(variable);
    } else if (auto call = std::dynamic_pointer_cast<CallExpression>(expr)) {
        visitCallExpression(call);
    } else if (auto array = std::dynamic_pointer_cast<ArrayExpression>(expr)) {
        visitArrayExpression(array);
    } else if (auto arrayAccess = std::dynamic_pointer_cast<ArrayAccessExpression>(expr)) {
        visitArrayAccessExpression(arrayAccess);
    } else if (auto memberAccess = std::dynamic_pointer_cast<MemberAccessExpression>(expr)) {
        visitMemberAccessExpression(memberAccess);
    } else {
        throw SemanticError("Unknown expression type");
    }
}

void SemanticAnalyzer::visitVarDeclaration(const std::shared_ptr<VarDeclarationStatement>& stmt) {
    // Check if variable is already defined in current scope
    if (current_scope->isDefined(stmt->name)) {
        std::stringstream ss;
        ss << "Variable '" << stmt->name << "' is already defined in this scope";
        throw SemanticError(ss.str());
    }
    
    // If there's an initializer, check it
    if (stmt->initializer != nullptr) {
        visit(stmt->initializer);
    }
    
    // Add variable to current scope
    current_scope->define(stmt->name, Symbol(Symbol::Type::VARIABLE, stmt->initializer != nullptr));
}

void SemanticAnalyzer::visitFunctionDeclaration(const std::shared_ptr<FunctionDeclaration>& stmt) {
    // Check if function is already defined in current scope
    if (current_scope->isDefined(stmt->name)) {
        std::stringstream ss;
        ss << "Function '" << stmt->name << "' is already defined in this scope";
        throw SemanticError(ss.str());
    }
    
    // Add function to current scope
    current_scope->define(stmt->name, Symbol(Symbol::Type::FUNCTION, true, stmt->parameters.size()));
    
    // Enter new scope for function body
    enterScope();
    
    // Set in_function flag
    bool previous_in_function = in_function;
    in_function = true;
    
    // Add parameters to the function scope
    for (const auto& param : stmt->parameters) {
        if (current_scope->isDefined(param)) {
            std::stringstream ss;
            ss << "Parameter '" << param << "' is already defined in function '" << stmt->name << "'";
            throw SemanticError(ss.str());
        }
        current_scope->define(param, Symbol(Symbol::Type::VARIABLE, true));
    }
    
    // Visit function body
    visit(stmt->body);
    
    // Restore in_function flag
    in_function = previous_in_function;
    
    // Exit function scope
    exitScope();
}

void SemanticAnalyzer::visitExpressionStatement(const std::shared_ptr<ExpressionStatement>& stmt) {
    visit(stmt->expression);
}

void SemanticAnalyzer::visitIfStatement(const std::shared_ptr<IfStatement>& stmt) {
    // Check condition
    visit(stmt->condition);
    
    // Check then branch
    enterScope();
    visit(stmt->then_branch);
    exitScope();
    
    // Check else branch if it exists
    if (stmt->else_branch != nullptr) {
        enterScope();
        visit(stmt->else_branch);
        exitScope();
    }
}

void SemanticAnalyzer::visitWhileStatement(const std::shared_ptr<WhileStatement>& stmt) {
    // Check condition
    visit(stmt->condition);
    
    // Check body
    enterScope();
    visit(stmt->body);
    exitScope();
}

void SemanticAnalyzer::visitReturnStatement(const std::shared_ptr<ReturnStatement>& stmt) {
    // Check that we're inside a function
    if (!in_function) {
        throw SemanticError("Cannot return from outside a function");
    }
    
    // Check return value if it exists
    if (stmt->value != nullptr) {
        visit(stmt->value);
    }
}

void SemanticAnalyzer::visitBlockStatement(const std::shared_ptr<BlockStatement>& stmt) {
    // Visit all statements in the block
    for (const auto& s : stmt->statements) {
        visit(s);
    }
}

void SemanticAnalyzer::visitPrintStatement(const std::shared_ptr<PrintStatement>& stmt) {
    visit(stmt->expression);
}

void SemanticAnalyzer::visitInputStatement(const std::shared_ptr<InputStatement>& stmt) {
    // Check if variable exists
    Symbol* symbol = current_scope->resolve(stmt->variable);
    if (symbol == nullptr) {
        std::stringstream ss;
        ss << "Variable '" << stmt->variable << "' is not defined";
        throw SemanticError(ss.str());
    }
    
    // Mark variable as initialized
    symbol->initialized = true;
}

void SemanticAnalyzer::visitLoopInStatement(const std::shared_ptr<LoopInStatement>& stmt) {
    // Check iterable expression
    visit(stmt->iterable);
    
    // Create new scope for loop body
    enterScope();
    
    // Add loop variable to scope
    current_scope->define(stmt->variable, Symbol(Symbol::Type::VARIABLE, true));
    
    // Visit loop body
    visit(stmt->body);
    
    // Exit loop scope
    exitScope();
}

void SemanticAnalyzer::visitLoopTimesStatement(const std::shared_ptr<LoopTimesStatement>& stmt) {
    // Check count expression
    visit(stmt->count);
    
    // Create new scope for loop body
    enterScope();
    
    // Visit loop body
    visit(stmt->body);
    
    // Exit loop scope
    exitScope();
}

void SemanticAnalyzer::visitBinaryExpression(const std::shared_ptr<BinaryExpression>& expr) {
    visit(expr->left);
    visit(expr->right);
    
    // If this is an assignment, check that the left side is a valid assignment target
    if (expr->op == TokenType::ASSIGN) {
        if (auto var = std::dynamic_pointer_cast<VariableExpression>(expr->left)) {
            // Variable assignment - check that variable exists
            Symbol* symbol = current_scope->resolve(var->name);
            if (symbol == nullptr) {
                std::stringstream ss;
                ss << "Variable '" << var->name << "' is not defined";
                throw SemanticError(ss.str());
            }
            
            // Mark variable as initialized
            symbol->initialized = true;
        } else if (auto array = std::dynamic_pointer_cast<ArrayAccessExpression>(expr->left)) {
            // Array element assignment - already checked in visit
        } else {
            throw SemanticError("Invalid assignment target");
        }
    }
}

void SemanticAnalyzer::visitUnaryExpression(const std::shared_ptr<UnaryExpression>& expr) {
    visit(expr->right);
}

void SemanticAnalyzer::visitLiteralExpression(const std::shared_ptr<LiteralExpression>& expr) {
    // Nothing to check for literals
}

void SemanticAnalyzer::visitVariableExpression(const std::shared_ptr<VariableExpression>& expr) {
    // Check if variable exists
    Symbol* symbol = current_scope->resolve(expr->name);
    if (symbol == nullptr) {
        std::stringstream ss;
        ss << "Variable '" << expr->name << "' is not defined";
        throw SemanticError(ss.str());
    }
    
    // Check if variable is initialized
    if (!symbol->initialized) {
        std::stringstream ss;
        ss << "Variable '" << expr->name << "' is not initialized";
        throw SemanticError(ss.str());
    }
}

void SemanticAnalyzer::visitCallExpression(const std::shared_ptr<CallExpression>& expr) {
    std::string callee_name = expr->callee;

    // Check for known built-in functions
    bool is_builtin_convert = (callee_name == "int" || callee_name == "float" || callee_name == "str" || callee_name == "bool");

    if (is_builtin_convert) {
        // Built-in conversion functions expect exactly one argument
        if (expr->arguments.size() != 1) {
            std::stringstream ss;
            ss << "Built-in function '" << callee_name << "' expects 1 argument, but got " 
               << expr->arguments.size();
            throw SemanticError(ss.str());
        }
    } else {
        // Check user-defined functions
        Symbol* symbol = current_scope->resolve(callee_name);
        if (symbol == nullptr) {
            std::stringstream ss;
            ss << "Function '" << callee_name << "' is not defined";
            throw SemanticError(ss.str());
        }
        
        // Check if it's a function
        if (symbol->type != Symbol::Type::FUNCTION) {
            std::stringstream ss;
            ss << "'" << callee_name << "' is not a function";
            throw SemanticError(ss.str());
        }
        
        // Check argument count for user-defined functions
        if (symbol->paramCount != expr->arguments.size()) {
            std::stringstream ss;
            ss << "Function '" << callee_name << "' expects " << symbol->paramCount
               << " arguments, but got " << expr->arguments.size();
            throw SemanticError(ss.str());
        }
    }
    
    // Check arguments (for both built-in and user-defined)
    for (const auto& arg : expr->arguments) {
        visit(arg);
    }
}

void SemanticAnalyzer::visitArrayExpression(const std::shared_ptr<ArrayExpression>& expr) {
    // Check all elements
    for (const auto& element : expr->elements) {
        visit(element);
    }
}

void SemanticAnalyzer::visitArrayAccessExpression(const std::shared_ptr<ArrayAccessExpression>& expr) {
    // Check array expression
    visit(expr->array);
    
    // Check index expression
    visit(expr->index);
}

void SemanticAnalyzer::visitMemberAccessExpression(const std::shared_ptr<MemberAccessExpression>& expr) {
    // Check object expression
    visit(expr->object);
    
    // We can't really check if the member exists at compile time
    // since we don't have type information for the object
}

void SemanticAnalyzer::printSymbolTable() const {
    if (current_scope == nullptr) {
        std::cout << "No symbol table available\n";
        return;
    }

    std::cout << "Symbol Table:\n";
    for (const auto& [name, symbol] : current_scope->symbols) {
        std::cout << "  " << name << ": ";
        if (symbol.type == Symbol::Type::VARIABLE) {
            std::cout << "VARIABLE";
            if (!symbol.initialized) {
                std::cout << " (uninitialized)";
            }
        } else {
            std::cout << "FUNCTION";
            std::cout << " (" << symbol.paramCount << " parameters)";
        }
        std::cout << "\n";
    }
}

} // namespace vypr 