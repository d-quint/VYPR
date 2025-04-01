#include "parser.h"
#include <sstream>
#include <iostream> // Add for debug prints

namespace vypr {

Parser::Parser(const std::vector<Token>& tokens, bool verbose)
    : tokens(tokens), current(0), verbose(verbose) {}

std::shared_ptr<Program> Parser::parse() {
    if (this->verbose) std::cout << "Starting parse()...\n"; // Debug
    auto program_node = program();
    if (this->verbose) std::cout << "Finished program(). Returning AST.\n"; // Debug
    return program_node;
}

Token Parser::peek() const {
    // Simple bounds check
    if (current >= tokens.size()) {
        // Return the last token if available (should be EOF), or a default EOF
        if (!tokens.empty()) {
             return tokens.back(); 
        }
        // Should ideally not happen if lexer always adds EOF, but return a synthetic one
        return Token(TokenType::EOF_TOKEN, -1, -1); 
    }
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    // Check bounds FIRST, then check for EOF token type
    return current >= tokens.size() || tokens[current].type == TokenType::EOF_TOKEN;
}

Token Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (const auto& type : types) {
        if (match(type)) {
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    
    throw error(peek(), message);
}

ParseError Parser::error(const Token& token, const std::string& message) {
    std::stringstream ss;
    ss << "Line " << token.line << ": " << message;
    return ParseError(ss.str());
}

void Parser::synchronize() {
    if (this->verbose) std::cout << "Entering synchronize().\n"; // Debug
    advance();
    while (!isAtEnd()) {
        if (this->verbose) std::cout << "synchronize() loop, current: " << peek().toString() << "\n"; // Debug
        if (previous().type == TokenType::NEWLINE) {
            if (this->verbose) std::cout << "synchronize() found NEWLINE, returning.\n"; // Debug
            return;
        }
        switch (peek().type) {
            case TokenType::VAR:
            case TokenType::FUNC:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::LOOP:
            case TokenType::RETURN:
            case TokenType::PRINT:
            case TokenType::INPUT:
                if (this->verbose) std::cout << "synchronize() found statement start, returning.\n"; // Debug
                return;
            default:
                break;
        }
        advance();
    }
    if (this->verbose) std::cout << "synchronize() reached end.\n"; // Debug
}

std::shared_ptr<Program> Parser::program() {
    if (this->verbose) std::cout << "Entering program().\n"; // Debug
    std::vector<StatementPtr> statements;
    while (!isAtEnd()) {
        if (this->verbose) std::cout << "program() loop start, current: " << peek().toString() << "\n"; // Debug
        if (match(TokenType::NEWLINE)) {
            if (this->verbose) std::cout << "program() skipped NEWLINE.\n"; // Debug
            continue;
        }
        if (this->verbose) std::cout << "program() calling declaration().\n"; // Debug
        statements.push_back(declaration());
        if (this->verbose) std::cout << "program() returned from declaration().\n"; // Debug
    }
    if (this->verbose) std::cout << "program() loop finished.\n"; // Debug
    return std::make_shared<Program>(statements);
}

StatementPtr Parser::declaration() {
    if (this->verbose) std::cout << "Entering declaration(), current: " << peek().toString() << "\n"; // Debug
    if (match(TokenType::VAR)) {
        return var_declaration();
    }
    
    if (match(TokenType::FUNC)) {
        return func_declaration();
    }
    
    if (this->verbose) std::cout << "declaration() calling statement().\n"; // Debug
    return statement();
}

StatementPtr Parser::var_declaration() {
    std::string name;
    if (match(TokenType::IDENTIFIER)) {
        name = std::get<std::string>(previous().value);
    } else {
        throw error(peek(), "Expected variable name.");
    }
    
    ExpressionPtr initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = expression();
    }
    
    match(TokenType::NEWLINE);  // Consume the newline
    return std::make_shared<VarDeclarationStatement>(name, initializer);
}

StatementPtr Parser::func_declaration() {
    std::string name;
    if (match(TokenType::IDENTIFIER)) {
        name = std::get<std::string>(previous().value);
    } else {
        throw error(peek(), "Expected function name.");
    }
    
    consume(TokenType::LPAREN, "Expected '(' after function name.");
    std::vector<std::string> params = parameters();
    consume(TokenType::RPAREN, "Expected ')' after parameters.");
    
    consume(TokenType::COLON, "Expected ':' after function declaration.");
    match(TokenType::NEWLINE);  // Consume the newline
    
    // Expect INDENT for function body
    consume(TokenType::INDENT, "Expected indented function body.");
    
    StatementPtr body = block();
    
    return std::make_shared<FunctionDeclaration>(name, params, body);
}

std::vector<std::string> Parser::parameters() {
    std::vector<std::string> params;
    
    if (!check(TokenType::RPAREN)) {
        do {
            if (match(TokenType::IDENTIFIER)) {
                params.push_back(std::get<std::string>(previous().value));
            } else {
                throw error(peek(), "Expected parameter name.");
            }
        } while (match(TokenType::COMMA));
    }
    
    return params;
}

StatementPtr Parser::statement() {
    if (this->verbose) std::cout << "Entering statement(), current: " << peek().toString() << "\n"; // Debug
    if (match(TokenType::IF)) {
        return if_statement();
    }
    
    if (match(TokenType::WHILE)) {
        return while_statement();
    }
    
    if (match(TokenType::LOOP)) {
        return loop_statement();
    }
    
    if (match(TokenType::RETURN)) {
        return return_statement();
    }
    
    if (match(TokenType::PRINT)) {
        if (this->verbose) std::cout << "statement() matched PRINT, calling print_statement().\n"; // Debug
        return print_statement();
    }
    
    if (match(TokenType::INPUT)) {
        return input_statement();
    }
    
    if (this->verbose) std::cout << "statement() calling expression_statement().\n"; // Debug
    return expression_statement();
}

StatementPtr Parser::if_statement() {
    ExpressionPtr condition = expression();
    
    consume(TokenType::COLON, "Expected ':' after if condition.");
    match(TokenType::NEWLINE);  // Consume the newline
    
    // Expect INDENT token
    consume(TokenType::INDENT, "Expected indented if body.");
    
    StatementPtr then_branch = block();
    StatementPtr else_branch = nullptr;
    
    // Check if there's an else branch
    if (match(TokenType::ELSE)) {
        if (match(TokenType::IF)) {
            // This is an "else if"
            else_branch = if_statement();
        } else {
            consume(TokenType::COLON, "Expected ':' after else.");
            match(TokenType::NEWLINE);  // Consume the newline
            
            // Expect INDENT token
            consume(TokenType::INDENT, "Expected indented else body.");
            
            else_branch = block();
        }
    }
    
    return std::make_shared<IfStatement>(condition, then_branch, else_branch);
}

StatementPtr Parser::while_statement() {
    ExpressionPtr condition = expression();
    
    consume(TokenType::COLON, "Expected ':' after while condition.");
    match(TokenType::NEWLINE);  // Consume the newline
    
    // Expect INDENT token
    consume(TokenType::INDENT, "Expected indented while body.");
    
    StatementPtr body = block();
    
    return std::make_shared<WhileStatement>(condition, body);
}

StatementPtr Parser::loop_statement() {
    // Check if it's a "loop N times" statement
    if (check(TokenType::INTEGER) || check(TokenType::IDENTIFIER)) {
        ExpressionPtr count = expression();
        
        consume(TokenType::TIMES, "Expected 'times' after count.");
        consume(TokenType::COLON, "Expected ':' after loop times.");
        match(TokenType::NEWLINE);  // Consume the newline
        
        // Expect INDENT token
        consume(TokenType::INDENT, "Expected indented loop body.");
        
        StatementPtr body = block();
        
        return std::make_shared<LoopTimesStatement>(count, body);
    }
    
    // Check if it's a "loop var in collection" statement
    if (match(TokenType::IDENTIFIER)) {
        std::string var_name = std::get<std::string>(previous().value);
        
        consume(TokenType::IN, "Expected 'in' after variable in loop.");
        ExpressionPtr iterable = expression();
        
        consume(TokenType::COLON, "Expected ':' after loop in.");
        match(TokenType::NEWLINE);  // Consume the newline
        
        // Expect INDENT token
        consume(TokenType::INDENT, "Expected indented loop body.");
        
        StatementPtr body = block();
        
        return std::make_shared<LoopInStatement>(var_name, iterable, body);
    }
    
    throw error(peek(), "Expected variable name or number after 'loop'.");
}

StatementPtr Parser::return_statement() {
    ExpressionPtr value = nullptr;
    
    if (!check(TokenType::NEWLINE)) {
        value = expression();
    }
    
    match(TokenType::NEWLINE);  // Consume the newline
    return std::make_shared<ReturnStatement>(value);
}

StatementPtr Parser::print_statement() {
    if (this->verbose) std::cout << "Entering print_statement().\n"; // Debug
    ExpressionPtr value = expression();
    if (this->verbose) std::cout << "print_statement() parsed expression, checking NEWLINE, current: " << peek().toString() << "\n"; // Debug
    match(TokenType::NEWLINE);  // Consume the newline
    if (this->verbose) std::cout << "print_statement() finished.\n"; // Debug
    return std::make_shared<PrintStatement>(value);
}

StatementPtr Parser::input_statement() {
    std::string var_name;
    
    if (match(TokenType::IDENTIFIER)) {
        var_name = std::get<std::string>(previous().value);
    } else {
        throw error(peek(), "Expected variable name after 'input'.");
    }
    
    match(TokenType::NEWLINE);  // Consume the newline
    return std::make_shared<InputStatement>(var_name);
}

StatementPtr Parser::expression_statement() {
    ExpressionPtr expr = expression();
    
    match(TokenType::NEWLINE);  // Consume the newline
    return std::make_shared<ExpressionStatement>(expr);
}

StatementPtr Parser::block() {
    std::vector<StatementPtr> statements;
    
    while (!check(TokenType::DEDENT) && !check(TokenType::EOF_TOKEN)) {
        if (match(TokenType::NEWLINE)) {
            continue;  // Skip empty lines
        }
        statements.push_back(declaration());
    }
    
    consume(TokenType::DEDENT, "Expected dedent at end of block.");
    return std::make_shared<BlockStatement>(statements);
}

ExpressionPtr Parser::expression() {
    return assignment();
}

ExpressionPtr Parser::assignment() {
    ExpressionPtr expr = logical_or();
    
    if (match(TokenType::ASSIGN)) {
        ExpressionPtr value = assignment();
        
        if (auto* var_expr = dynamic_cast<VariableExpression*>(expr.get())) {
            return std::make_shared<BinaryExpression>(
                std::make_shared<VariableExpression>(var_expr->name),
                TokenType::ASSIGN,
                value
            );
        } else if (auto* array_access = dynamic_cast<ArrayAccessExpression*>(expr.get())) {
            return std::make_shared<BinaryExpression>(
                std::make_shared<ArrayAccessExpression>(array_access->array, array_access->index),
                TokenType::ASSIGN,
                value
            );
        }
        
        throw error(previous(), "Invalid assignment target.");
    }
    
    return expr;
}

ExpressionPtr Parser::logical_or() {
    ExpressionPtr expr = logical_and();
    
    while (match(TokenType::OR)) {
        TokenType op = previous().type;
        ExpressionPtr right = logical_and();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

ExpressionPtr Parser::logical_and() {
    ExpressionPtr expr = equality();
    
    while (match(TokenType::AND)) {
        TokenType op = previous().type;
        ExpressionPtr right = equality();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

ExpressionPtr Parser::equality() {
    ExpressionPtr expr = comparison();
    
    while (match({TokenType::EQUAL, TokenType::NOT_EQUAL})) {
        TokenType op = previous().type;
        ExpressionPtr right = comparison();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

ExpressionPtr Parser::comparison() {
    ExpressionPtr expr = term();
    
    while (match({TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL})) {
        TokenType op = previous().type;
        ExpressionPtr right = term();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

ExpressionPtr Parser::term() {
    ExpressionPtr expr = factor();
    
    while (match({TokenType::PLUS, TokenType::MINUS, TokenType::CONCAT})) {
        TokenType op = previous().type;
        ExpressionPtr right = factor();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

ExpressionPtr Parser::factor() {
    ExpressionPtr expr = unary();
    
    while (match({TokenType::MULTIPLY, TokenType::DIVIDE})) {
        TokenType op = previous().type;
        ExpressionPtr right = unary();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

ExpressionPtr Parser::unary() {
    if (match({TokenType::MINUS, TokenType::NOT})) {
        TokenType op = previous().type;
        ExpressionPtr right = unary();
        return std::make_shared<UnaryExpression>(op, right);
    }
    
    return call();
}

ExpressionPtr Parser::call() {
    ExpressionPtr expr = primary();
    
    while (true) {
        if (match(TokenType::LPAREN)) {
            expr = finish_call(expr);
        } else if (match(TokenType::LBRACKET)) {
            ExpressionPtr index = expression();
            consume(TokenType::RBRACKET, "Expected ']' after array index.");
            expr = std::make_shared<ArrayAccessExpression>(expr, index);
        } else if (match(TokenType::DOT)) {
            if (match(TokenType::IDENTIFIER)) {
                std::string member = std::get<std::string>(previous().value);
                expr = std::make_shared<MemberAccessExpression>(expr, member);
            } else {
                throw error(peek(), "Expected property name after '.'.");
            }
        } else {
            break;
        }
    }
    
    return expr;
}

ExpressionPtr Parser::primary() {
    if (match(TokenType::BOOLEAN)) {
        return std::make_shared<LiteralExpression>(std::get<bool>(previous().value));
    }
    
    if (match(TokenType::INTEGER)) {
        return std::make_shared<LiteralExpression>(std::get<int>(previous().value));
    }
    
    if (match(TokenType::FLOAT)) {
        return std::make_shared<LiteralExpression>(std::get<double>(previous().value));
    }
    
    if (match(TokenType::STRING)) {
        return std::make_shared<LiteralExpression>(std::get<std::string>(previous().value));
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_shared<VariableExpression>(std::get<std::string>(previous().value));
    }
    
    if (match(TokenType::LPAREN)) {
        ExpressionPtr expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression.");
        return expr;
    }
    
    if (match(TokenType::LBRACKET)) {
        std::vector<ExpressionPtr> elements;
        
        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
            } while (match(TokenType::COMMA));
        }
        
        consume(TokenType::RBRACKET, "Expected ']' after array elements.");
        return std::make_shared<ArrayExpression>(elements);
    }
    
    throw error(peek(), "Expected expression.");
}

ExpressionPtr Parser::finish_call(ExpressionPtr callee) {
    std::vector<ExpressionPtr> args = arguments();
    consume(TokenType::RPAREN, "Expected ')' after arguments.");
    
    std::string function_name;
    if (auto* var = dynamic_cast<VariableExpression*>(callee.get())) {
        function_name = var->name;
    } else {
        throw error(previous(), "Expected function name.");
    }
    
    return std::make_shared<CallExpression>(function_name, args);
}

std::vector<ExpressionPtr> Parser::arguments() {
    std::vector<ExpressionPtr> args;
    
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(expression());
        } while (match(TokenType::COMMA));
    }
    
    return args;
}

} // namespace vypr
 