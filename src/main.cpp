#include "compiler.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdlib> // Include for system()

using namespace vypr;

void printUsage() {
    std::cout << "Vypr Compiler - Translates Vypr (.vy) files to Python\n";
    std::cout << "Usage: vypr [options] <source_file.vy>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -v, -verbose   Show compilation progress and debugging information\n";
    std::cout << "  -o <filename>  Specify output executable name (without extension)\n";
    std::cout << "  -h, --help     Show this help message\n";
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    std::string output_file;
    std::string source_file;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing output filename after -o\n";
                printUsage();
                return 1;
            }
            output_file = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        } else {
            source_file = arg;
        }
    }
    
    if (source_file.empty()) {
        std::cerr << "Error: No source file specified\n";
        printUsage();
        return 1;
    }
    
    // Check file extension
    if (source_file.substr(source_file.find_last_of(".") + 1) != "vy") {
        std::cerr << "Error: Source file must have .vy extension\n";
        return 1;
    }
    
    // If no output file specified, use source file name without extension
    if (output_file.empty()) {
        output_file = source_file.substr(0, source_file.find_last_of("."));
    }
    
    try {
        // Read source file
        std::ifstream file(source_file);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open source file: " << source_file << "\n";
            return 1;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        file.close();
        
        // Compile
        Compiler compiler;
        std::string py_file = output_file + ".py"; // Keep track of the .py filename
        compiler.compile(source, output_file, verbose); // Pass base output name

        if (!verbose) {
            std::cout << "Compilation successful!\n";
            std::cout << "Output files:\n";
            std::cout << "  - " << py_file << "\n";
            std::cout << "  - " << output_file << ".bat\n";
            
            // Attempt to run the generated Python file
            std::cout << "\nAttempting to run generated Python script...\n";
            std::cout << "\n==================== Program Output Start ====================\n\n";
            
            std::string command = "python " + py_file;
            int return_code = system(command.c_str());
            
            std::cout << "\n==================== Program Output End ======================\n\n";
            
            if (return_code != 0) {
                std::cerr << "Warning: Python script execution might have failed (return code: " 
                          << return_code << "). Ensure 'python' is in your PATH.\n";
            }
        } else {
             std::cout << "\nVerbose mode: Skipping automatic execution.\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
} 