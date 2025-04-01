#ifndef VYPR_COMPILER_H
#define VYPR_COMPILER_H

#include <string>
#include <vector>
#include <memory>
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "ir_generator.h"
#include "code_generator.h"
#include "exceptions.h"

namespace vypr {

class CompilationError : public std::runtime_error {
public:
    explicit CompilationError(const std::string& message) : std::runtime_error(message) {}
};

class Compiler {
public:
    Compiler(bool verbose = false);
    
    // Compile a Vypr source file to a Python output file
    void compile(const std::string& sourceFile, const std::string& outputFile, bool verbose);
    
    // Compile and run a Vypr program (generates Python and executes it)
    void compileAndRun(const std::string& sourceFile, const std::string& outputExe = "");
    
private:
    bool verbose;
    
    // Read source file content
    std::string readSourceFile(const std::string& sourceFile);
    
    // Create executable batch file on Windows
    void createExecutable(const std::string& pythonFile, const std::string& outputExe);
    
    // Log messages when verbose mode is on
    void log(const std::string& message);
};

} // namespace vypr

#endif // VYPR_COMPILER_H 