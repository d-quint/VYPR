#ifndef VYPR_TOKEN_H
#define VYPR_TOKEN_H

#include <string>
#include <variant>
#include <sstream>

namespace vypr {

enum class TokenType {
    // Keywords
    VAR,
    FUNC,
    RETURN,
    IF,
    ELSE,
    WHILE,
    LOOP,
    IN,
    TIMES,
    PRINT,
    INPUT,
    
    // Data types
    STRING,
    INTEGER,
    FLOAT,
    BOOLEAN,
    IDENTIFIER,
    
    // Operators
    PLUS,         // +
    MINUS,        // -
    MULTIPLY,     // *
    DIVIDE,       // /
    CONCAT,       // ^
    ASSIGN,       // =
    
    // Comparison operators
    EQUAL,        // ==
    NOT_EQUAL,    // !=
    GREATER,      // >
    LESS,         // <
    GREATER_EQUAL, // >=
    LESS_EQUAL,   // <=
    
    // Logical operators
    AND,          // &&
    OR,           // ||
    NOT,          // !
    
    // Delimiters
    LPAREN,       // (
    RPAREN,       // )
    LBRACKET,     // [
    RBRACKET,     // ]
    COMMA,        // ,
    DOT,          // .
    COLON,        // :
    NEWLINE,      // Represents a significant newline
    INDENT,       // Increase in indentation level
    DEDENT,       // Decrease in indentation level
    EOF_TOKEN,    // End Of File
    UNKNOWN       // Unknown token type
};

// Helper function to convert token type to string
std::string tokenTypeToString(TokenType type);

using TokenValue = std::variant<std::monostate, std::string, int, double, bool>;

class Token {
public:
    TokenType type;
    std::variant<std::string, int, double, bool> value;
    int line;
    int column;
    
    Token(TokenType type, int line, int column)
        : type(type), line(line), column(column) {}
    
    Token(TokenType type, const std::string& value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
    
    Token(TokenType type, int value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
    
    Token(TokenType type, double value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
    
    Token(TokenType type, bool value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
    
    std::string toString() const {
        std::stringstream ss;
        ss << tokenTypeToString(type);
        
        if (value.index() != std::variant_npos) {
            ss << " (";
            if (std::holds_alternative<std::string>(value)) {
                ss << "\"" << std::get<std::string>(value) << "\"";
            } else if (std::holds_alternative<int>(value)) {
                ss << std::get<int>(value);
            } else if (std::holds_alternative<double>(value)) {
                ss << std::get<double>(value);
            } else if (std::holds_alternative<bool>(value)) {
                ss << (std::get<bool>(value) ? "true" : "false");
            }
            ss << ")";
        }
        
        ss << " at line " << line << ", column " << column;
        return ss.str();
    }
};

} // namespace vypr

#endif // VYPR_TOKEN_H 