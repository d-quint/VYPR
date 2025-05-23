#include "code_generator.h"
#include <iostream>
#include <map>
#include <stdexcept>

namespace vypr {

CodeGenerator::CodeGenerator(bool verbose) : verbose(verbose) {
    // Initialize opcode handlers
    opcodeHandlers[IROpCode::LOAD_CONST] = &CodeGenerator::handleLoadConst;
    opcodeHandlers[IROpCode::LOAD_VAR] = &CodeGenerator::handleLoadVar;
    opcodeHandlers[IROpCode::STORE_VAR] = &CodeGenerator::handleStoreVar;
    opcodeHandlers[IROpCode::BINARY_OP] = &CodeGenerator::handleBinaryOp;
    opcodeHandlers[IROpCode::UNARY_OP] = &CodeGenerator::handleUnaryOp;
    opcodeHandlers[IROpCode::CALL] = &CodeGenerator::handleCall;
    opcodeHandlers[IROpCode::RETURN] = &CodeGenerator::handleReturn;
    opcodeHandlers[IROpCode::PRINT] = &CodeGenerator::handlePrint;
    opcodeHandlers[IROpCode::INPUT] = &CodeGenerator::handleInput;
    opcodeHandlers[IROpCode::ARRAY_NEW] = &CodeGenerator::handleArrayNew;
    opcodeHandlers[IROpCode::ARRAY_GET] = &CodeGenerator::handleArrayGet;
    opcodeHandlers[IROpCode::ARRAY_SET] = &CodeGenerator::handleArraySet;
    opcodeHandlers[IROpCode::MEMBER_GET] = &CodeGenerator::handleMemberGet;
    opcodeHandlers[IROpCode::CONVERT] = &CodeGenerator::handleConvert;
    opcodeHandlers[IROpCode::NOP] = &CodeGenerator::handleNop;
}

void CodeGenerator::generate(const std::vector<IRFunction>& functions, const std::string& outputFile) {
    // Open output file
    outFile.open(outputFile);
    if (!outFile.is_open()) {
        throw std::runtime_error("Could not open output file: " + outputFile);
    }
    
    if (verbose) {
        std::cout << "Generating Python code to " << outputFile << std::endl;
    }
    
    // Write Python file header
    writeHeader();
    
    // Write functions
    for (const auto& function : functions) {
        writeFunction(function);
    }
    
    // Add main execution if there is a __main__ function
    outFile << "\n# Execute main function if this is the main module\n";
    outFile << "if __name__ == \"__main__\":\n";
    outFile << "    __main__()\n";
    
    // Close output file
    outFile.close();
    
    if (verbose) {
        std::cout << "Code generation complete." << std::endl;
    }
}

void CodeGenerator::writeHeader() {
    outFile << "#!/usr/bin/env python3\n";
    outFile << "# Generated by Vypr Compiler\n\n";
    
    // Import runtime libraries
    outFile << "import sys\n\n";
    
    // Define runtime helper functions
    outFile << "# Runtime helper functions\n";
    outFile << "def _vypr_concat(a, b):\n";
    outFile << "    return str(a) + str(b)\n\n";
    
    outFile << "def _vypr_input(prompt=\"\"):\n";
    outFile << "    if prompt:\n";
    outFile << "        sys.stdout.write(prompt)\n";
    outFile << "        sys.stdout.flush()\n";
    outFile << "    return input()\n\n";
}

void CodeGenerator::writeFunction(const IRFunction& function) {
    // Write function header
    outFile << "def " << function.name << "(";
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        if (i > 0) outFile << ", ";
        outFile << function.parameters[i];
    }
    outFile << "):\n";

    // Build label map (Label Name -> Instruction Index)
    std::map<std::string, int> label_map;
    for (size_t i = 0; i < function.instructions.size(); ++i) {
        const auto& instr = function.instructions[i];
        if (instr.opcode == IROpCode::LABEL) {
            if (label_map.count(instr.operands[0])) {
                 throw std::runtime_error("Duplicate label found in IR function '" + function.name + "': " + instr.operands[0]);
            }
            label_map[instr.operands[0]] = static_cast<int>(i);
        }
    }

    // Initialize Python program counter
    outFile << getIndent(1) << "_pc = 0\n";
    // Start simulation loop
    outFile << getIndent(1) << "while True:\n";

    if (function.instructions.empty()) {
        // Handle empty function body
        outFile << getIndent(2) << "pass # Empty function\n";
        outFile << getIndent(2) << "break\n";
    } else {
        // Generate if/elif chain for instruction dispatch (only if instructions exist)
        for (size_t i = 0; i < function.instructions.size(); ++i) {
            const auto& instr = function.instructions[i];
            std::string current_block_indent = getIndent(2); // Indentation for if/elif _pc == N:
            std::string current_code_indent = getIndent(3); // Indentation for code inside the block

            // Start if/elif block for this instruction index
            if (i == 0) {
                outFile << current_block_indent << "if _pc == " << i << ":\n";
            } else {
                outFile << current_block_indent << "elif _pc == " << i << ":\n";
            }

            // Generate code based on opcode
            bool pc_increment_handled = false; // Track if jump/return handles _pc update

            switch (instr.opcode) {
                case IROpCode::LABEL:
                    outFile << current_code_indent << "# LABEL " << instr.operands[0] << "\n";
                    break;
    
                case IROpCode::JUMP: {
                    std::string target_label = instr.operands[0];
                    if (label_map.count(target_label)) {
                        outFile << current_code_indent << "_pc = " << label_map[target_label] << "\n";
                        pc_increment_handled = true;
                    } else {
                         throw std::runtime_error("Undefined label referenced in JUMP: " + target_label);
                    }
                    break;
                }
    
                case IROpCode::JUMP_IF_FALSE: {
                    std::string condition = instr.operands[0];
                    std::string target_label = instr.operands[1];
                    if (label_map.count(target_label)) {
                        outFile << current_code_indent << "if not " << condition << ":\n";
                        outFile << current_code_indent << getIndent(1) << "_pc = " << label_map[target_label] << "\n"; // Jump
                        outFile << current_code_indent << "else:\n";
                        outFile << current_code_indent << getIndent(1) << "_pc += 1\n"; // Go to next instruction
                        pc_increment_handled = true;
                    } else {
                         throw std::runtime_error("Undefined label referenced in JUMP_IF_FALSE: " + target_label);
                    }
                    break;
                }
    
                 case IROpCode::JUMP_IF_TRUE: {
                    std::string condition = instr.operands[0];
                    std::string target_label = instr.operands[1];
                     if (label_map.count(target_label)) {
                        outFile << current_code_indent << "if " << condition << ":\n";
                        outFile << current_code_indent << getIndent(1) << "_pc = " << label_map[target_label] << "\n"; // Jump
                        outFile << current_code_indent << "else:\n";
                        outFile << current_code_indent << getIndent(1) << "_pc += 1\n"; // Go to next instruction
                        pc_increment_handled = true;
                     } else {
                         throw std::runtime_error("Undefined label referenced in JUMP_IF_TRUE: " + target_label);
                     }
                    break;
                }
    
                case IROpCode::RETURN:
                    if (instr.operands.empty()) {
                        outFile << current_code_indent << "return\n";
                    } else {
                        outFile << current_code_indent << "return " << instr.operands[0] << "\n";
                    }
                    outFile << current_code_indent << "break # Exit loop after return\n";
                    pc_increment_handled = true;
                    break;
                // ... (cases for other instructions) ...
                case IROpCode::LOAD_CONST: outFile << current_code_indent << handleLoadConst(instr) << "\n"; break;
                case IROpCode::LOAD_VAR:   outFile << current_code_indent << handleLoadVar(instr) << "\n"; break;
                case IROpCode::STORE_VAR:  outFile << current_code_indent << handleStoreVar(instr) << "\n"; break;
                case IROpCode::BINARY_OP:  outFile << current_code_indent << handleBinaryOp(instr) << "\n"; break;
                case IROpCode::UNARY_OP:   outFile << current_code_indent << handleUnaryOp(instr) << "\n"; break;
                case IROpCode::CALL:       outFile << current_code_indent << handleCall(instr) << "\n"; break;
                case IROpCode::PRINT:      outFile << current_code_indent << handlePrint(instr) << "\n"; break;
                case IROpCode::INPUT:      outFile << current_code_indent << handleInput(instr) << "\n"; break;
                case IROpCode::ARRAY_NEW:  outFile << current_code_indent << handleArrayNew(instr) << "\n"; break;
                case IROpCode::ARRAY_GET:  outFile << current_code_indent << handleArrayGet(instr) << "\n"; break;
                case IROpCode::ARRAY_SET:  outFile << current_code_indent << handleArraySet(instr) << "\n"; break;
                case IROpCode::MEMBER_GET: outFile << current_code_indent << handleMemberGet(instr) << "\n"; break;
                case IROpCode::CONVERT:    outFile << current_code_indent << handleConvert(instr) << "\n"; break;
                case IROpCode::NOP:        outFile << current_code_indent << handleNop(instr) << "\n"; break;

                default:
                     throw std::runtime_error("Unsupported IR opcode encountered during Python code generation: OpCode " + std::to_string(static_cast<int>(instr.opcode)));
            }

            // Increment _pc for the next cycle if not handled by jump/return
            if (!pc_increment_handled) {
                 outFile << current_code_indent << "_pc += 1\n";
            }
        }

        // Add final else block for the while loop to catch runaway _pc (only if instructions exist)
        outFile << getIndent(2) << "else:\n";
        outFile << getIndent(3) << "# Instruction pointer out of bounds or loop finished\n";
        outFile << getIndent(3) << "break\n";
    }

    outFile << "\n"; // Newline after function definition
}

std::string CodeGenerator::handleLoadConst(const IRInstruction& instruction) {
    // Check if the constant is a string and needs quotes
    // This is a basic check; IR generator should ideally provide type info
    // or escape strings appropriately.
    std::string value = instruction.operands[1];
    if (!value.empty() && value.front() == '\"' && value.back() == '\"') {
         // Already looks like a Python string literal
    } else if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
         // Already looks like a Python string literal
    } else {
        // Attempt to detect if it's non-numeric to add quotes
        bool is_numeric = true;
        bool has_dot = false;
        if (value.empty()) is_numeric = false;
        for (size_t i = 0; i < value.length(); ++i) {
            if (i == 0 && value[i] == '-') continue; // Allow leading minus
            if (value[i] == '.' && !has_dot) { has_dot = true; continue; } // Allow one dot
            if (!std::isdigit(value[i])) {
                is_numeric = false;
                break;
            }
        }
         // Basic keywords check (true/false/None might be loaded as constants)
        if (value == "true") value = "True";
        else if (value == "false") value = "False";
        // Add more keywords if needed (e.g., None)
        else if (!is_numeric) {
            // Assuming non-numeric non-keyword needs quotes - this might be fragile
            // Ideally, IR should tag constants with types.
            // Escaping quotes within the string is also not handled here.
            value = "\\\"" + value + "\\\""; // Basic quoting
        }
    }
    return instruction.operands[0] + " = " + value;
}

std::string CodeGenerator::handleLoadVar(const IRInstruction& instruction) {
    return instruction.operands[0] + " = " + instruction.operands[1];
}

std::string CodeGenerator::handleStoreVar(const IRInstruction& instruction) {
    return instruction.operands[0] + " = " + instruction.operands[1];
}

std::string CodeGenerator::handleBinaryOp(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string left = instruction.operands[1];
    std::string op = instruction.operands[2];
    std::string right = instruction.operands[3];
    
    // Handle string concatenation specially
    if (op == "^") {
        return result + " = _vypr_concat(" + left + ", " + right + ")";
    }
    
    // Handle logical operators
    if (op == "&&") {
        op = "and";
    } else if (op == "||") {
        op = "or";
    } else if (op == "!") {
        op = "not";
    }
    
    return result + " = " + left + " " + op + " " + right;
}

std::string CodeGenerator::handleUnaryOp(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string op = instruction.operands[1];
    std::string operand = instruction.operands[2];
    
    // Convert logical not
    if (op == "!") {
        op = "not ";
    }
    
    return result + " = " + op + operand;
}

std::string CodeGenerator::handleCall(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string function = instruction.operands[1];
    std::string args = instruction.operands.size() > 2 ? instruction.operands[2] : "";
    
    return result + " = " + function + "(" + args + ")";
}

std::string CodeGenerator::handleReturn(const IRInstruction& instruction) {
    if (instruction.operands.empty()) {
        return "return";
    } else {
        return "return " + instruction.operands[0];
    }
}

std::string CodeGenerator::handlePrint(const IRInstruction& instruction) {
    return "print(" + instruction.operands[0] + ")";
}

std::string CodeGenerator::handleInput(const IRInstruction& instruction) {
    return instruction.operands[0] + " = _vypr_input()";
}

std::string CodeGenerator::handleArrayNew(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string elements = instruction.operands.size() > 1 ? instruction.operands[1] : "";
    
    return result + " = [" + elements + "]";
}

std::string CodeGenerator::handleArrayGet(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string array = instruction.operands[1];
    std::string index = instruction.operands[2];
    
    return result + " = " + array + "[" + index + "]";
}

std::string CodeGenerator::handleArraySet(const IRInstruction& instruction) {
    std::string array = instruction.operands[0];
    std::string index = instruction.operands[1];
    std::string value = instruction.operands[2];
    
    return array + "[" + index + "] = " + value;
}

std::string CodeGenerator::handleMemberGet(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string object = instruction.operands[1];
    std::string member = instruction.operands[2];
    
    // Special handling for array length
    if (member == "length") {
        return result + " = len(" + object + ")";
    }
    
    return result + " = " + object + "." + member;
}

std::string CodeGenerator::handleConvert(const IRInstruction& instruction) {
    std::string result = instruction.operands[0];
    std::string target_type = instruction.operands[1]; // e.g., "int", "float", "str", "bool"
    std::string source = instruction.operands[2];
    
    // Map Vypr type names to Python type names if necessary (they match here)
    std::string python_type = target_type; 
    
    return result + " = " + python_type + "(" + source + ")";
}

std::string CodeGenerator::handleNop([[maybe_unused]] const IRInstruction& instruction) {
    return "pass";
}

std::string CodeGenerator::getIndent(int level) const {
    return std::string(level * 4, ' ');
}

std::string CodeGenerator::getPythonOperator(TokenType op) const {
    switch (op) {
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::MULTIPLY: return "*";
        case TokenType::DIVIDE: return "/";
        case TokenType::CONCAT: return "_vypr_concat";
        case TokenType::EQUAL: return "==";
        case TokenType::NOT_EQUAL: return "!=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::AND: return "and";
        case TokenType::OR: return "or";
        case TokenType::NOT: return "not";
        default: return "?";
    }
}

} // namespace vypr 