#include "token.h"

namespace vypr {

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        // Keywords
        case TokenType::VAR: return "VAR";
        case TokenType::FUNC: return "FUNC";
        case TokenType::RETURN: return "RETURN";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::LOOP: return "LOOP";
        case TokenType::IN: return "IN";
        case TokenType::TIMES: return "TIMES";
        case TokenType::PRINT: return "PRINT";
        case TokenType::INPUT: return "INPUT";
        
        // Data types
        case TokenType::STRING: return "STRING";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::BOOLEAN: return "BOOLEAN";
        
        // Operators
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::CONCAT: return "CONCAT";
        case TokenType::ASSIGN: return "ASSIGN";
        
        // Comparison operators
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        
        // Logical operators
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        
        // Delimiters
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::NEWLINE: return "NEWLINE";
        
        // Other
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::UNKNOWN: return "UNKNOWN";
        
        default: return "<INVALID_TOKEN_TYPE>";
    }
}

} // namespace vypr 