#ifndef VYPR_EXCEPTIONS_H
#define VYPR_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace vypr {

// Base class for compiler errors
class CompileError : public std::runtime_error {
public:
    explicit CompileError(const std::string& message)
        : std::runtime_error("Compile Error: " + message) {}
};

// Specific error types (can add more later if needed)
class LexerError : public CompileError {
public:
    explicit LexerError(const std::string& message)
        : CompileError("Lexer Error: " + message) {}
};

class ParseError : public CompileError {
public:
    explicit ParseError(const std::string& message)
        : CompileError("Parse Error: " + message) {}
};

class SemanticError : public CompileError {
public:
    explicit SemanticError(const std::string& message)
        : CompileError("Semantic Error: " + message) {}
};

} // namespace vypr

#endif // VYPR_EXCEPTIONS_H 