#include "compiler.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cstring>

namespace vypr {

Compiler::Compiler(bool verbose) : verbose(verbose) {
    if (verbose) {
        std::cout << "Compiler initialized in verbose mode\n";
    }
}

void Compiler::compile(const std::string& source, const std::string& output_file, bool verbose) {
    try {
        // Lexical Analysis
        if (verbose) {
            std::cout << "\n=== Lexical Analysis ===\n";
        }
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        
        if (verbose) {
            std::cout << "Tokens found (" << tokens.size() << "):\n";
            for (size_t i = 0; i < tokens.size(); ++i) {
                const auto& token = tokens[i];
                std::cout << "  " << i << ": " << token.toString() << "\n";
            }
            std::cout << "\n";
        }
        
        // Syntax Analysis
        if (verbose) {
            std::cout << "=== Syntax Analysis ===\n";
        }
        Parser parser(tokens, verbose);
        std::shared_ptr<Program> ast = parser.parse();
        
        if (verbose) {
            std::cout << "Abstract Syntax Tree:\n";
            ast->print(std::cout, 2);  // Print with 2 spaces indentation
            std::cout << "\n";
        }
        
        // Semantic Analysis
        if (verbose) {
            std::cout << "=== Semantic Analysis ===\n";
        }
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast);
        
        if (verbose) {
            std::cout << "Symbol Table:\n";
            analyzer.printSymbolTable();
            std::cout << "\n";
        }
        
        // IR Generation
        if (verbose) {
            std::cout << "=== Intermediate Representation ===\n";
        }
        IRGenerator ir_gen;
        std::vector<IRFunction> functions = ir_gen.generate(ast);
        
        if (verbose) {
            std::cout << "IR Instructions (" << functions.size() << "):\n";
            for (const auto& function : functions) {
                std::cout << "  Function: " << function.name << "\n";
                for (size_t i = 0; i < function.instructions.size(); ++i) {
                    std::cout << "    " << i << ": " << function.instructions[i].toString() << "\n";
                }
            }
            std::cout << "\n";
        }
        
        // Code Generation
        if (verbose) {
            std::cout << "=== Code Generation ===\n";
        }
        CodeGenerator code_gen;
        std::string py_file = output_file + ".py";
        code_gen.generate(functions, py_file);
        
        // Write output batch file
        std::string bat_file = output_file + ".bat";
        std::ofstream bat_out(bat_file);
        bat_out << "@echo off\n";
        bat_out << "python " << py_file << "\n";
        bat_out << "pause\n";
        bat_out.close();
        
        if (verbose) {
            std::cout << "=== Output Files ===\n";
            std::cout << "Generated files:\n";
            std::cout << "  - " << py_file << "\n";
            std::cout << "  - " << bat_file << "\n";
        }
        
    } catch (const std::exception& e) {
        throw CompileError(e.what());
    }
}

void Compiler::compileAndRun(const std::string& sourceFile, const std::string& outputExe) {
    // Determine output Python file name
    std::string baseName = sourceFile.substr(0, sourceFile.find_last_of("."));
    std::string pyFile = baseName + ".py";
    
    // Compile Vypr to Python
    compile(sourceFile, pyFile, verbose);
    
    // Create executable batch file if requested
    if (!outputExe.empty()) {
        createExecutable(pyFile, outputExe);
    }
    
    log("Compilation successful!");
}

std::string Compiler::readSourceFile(const std::string& sourceFile) {
    std::ifstream file(sourceFile);
    if (!file.is_open()) {
        throw CompilationError("Could not open source file: " + sourceFile);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

void Compiler::createExecutable(const std::string& pythonFile, const std::string& outputExe) {
    log("Creating executable: " + outputExe);
    
    // Create a batch file for Windows
    std::ofstream batchFile(outputExe + ".bat");
    if (!batchFile.is_open()) {
        throw CompilationError("Could not create executable batch file: " + outputExe + ".bat");
    }
    
    // Write batch commands
    batchFile << "@echo off\n";
    batchFile << "python \"" << std::filesystem::absolute(pythonFile).string() << "\" %*\n";
    batchFile.close();
    
    log("Executable created: " + outputExe + ".bat");
}

void Compiler::log(const std::string& message) {
    if (verbose) {
        std::cout << "[VYPR] " << message << std::endl;
    }
}

} // namespace vypr 