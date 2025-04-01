#ifndef VYPR_PARSER_H
#define VYPR_PARSER_H

#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include "token.h"
#include "ast.h"
#include "exceptions.h"

namespace vypr {

class Parser {
public:
    Parser(const std::vector<Token>& tokens, bool verbose);
    std::shared_ptr<Program> parse();

private:
    std::vector<Token> tokens;
    size_t current;
    bool verbose;

    // Helper methods
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    ParseError error(const Token& token, const std::string& message);
    void synchronize();

    // Parsing methods for grammar rules
    std::shared_ptr<Program> program();
    StatementPtr declaration();
    StatementPtr var_declaration();
    StatementPtr func_declaration();
    std::vector<std::string> parameters();
    StatementPtr statement();
    StatementPtr if_statement();
    StatementPtr while_statement();
    StatementPtr loop_statement();
    StatementPtr return_statement();
    StatementPtr print_statement();
    StatementPtr input_statement();
    StatementPtr expression_statement();
    StatementPtr block();
    
    ExpressionPtr expression();
    ExpressionPtr assignment();
    ExpressionPtr logical_or();
    ExpressionPtr logical_and();
    ExpressionPtr equality();
    ExpressionPtr comparison();
    ExpressionPtr term();
    ExpressionPtr factor();
    ExpressionPtr unary();
    ExpressionPtr call();
    ExpressionPtr primary();
    ExpressionPtr finish_call(ExpressionPtr callee);
    std::vector<ExpressionPtr> arguments();
};

} // namespace vypr

#endif // VYPR_PARSER_H 