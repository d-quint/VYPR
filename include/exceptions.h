#pragma once

#include <stdexcept>
#include <string>

namespace vypr {

// Base class for all compiler-related errors
class CompileError : public std::runtime_error {
public:
    explicit CompileError(const std::string& message)
        : std::runtime_error(message) {}
};

// Lexer errors
class LexerError : public CompileError {
public:
    explicit LexerError(const std::string& message)
        : CompileError(message) {}
};

// Parser errors
class ParseError : public CompileError {
public:
    explicit ParseError(const std::string& message)
        : CompileError(message) {}
};

// Semantic analysis errors
class SemanticError : public CompileError {
public:
    explicit SemanticError(const std::string& message)
        : CompileError(message) {}
};

// Intermediate Representation errors
class IRError : public CompileError {
public:
    explicit IRError(const std::string& message)
        : CompileError(message) {}
};

// Code generation errors
class CodeGenError : public CompileError {
public:
    explicit CodeGenError(const std::string& message)
        : CompileError(message) {}
};

} // namespace vypr 