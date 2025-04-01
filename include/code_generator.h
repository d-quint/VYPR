#ifndef VYPR_CODE_GENERATOR_H
#define VYPR_CODE_GENERATOR_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include "ir_generator.h"

namespace vypr {

class CodeGenerator {
public:
    CodeGenerator(bool verbose = false);
    
    // Generate Python code from IR
    void generate(const std::vector<IRFunction>& functions, const std::string& outputFile);
    
private:
    bool verbose;
    std::ofstream outFile;
    using HandlerFunc = std::string (CodeGenerator::*)(const IRInstruction&);
    std::unordered_map<IROpCode, HandlerFunc> opcodeHandlers;
    
    // Helper methods
    void writeHeader();
    void writeFunction(const IRFunction& function);
    void writeInstruction(const IRInstruction& instruction);
    
    // Specific IR instruction handlers
    std::string handleLoadConst(const IRInstruction& instruction);
    std::string handleLoadVar(const IRInstruction& instruction);
    std::string handleStoreVar(const IRInstruction& instruction);
    std::string handleBinaryOp(const IRInstruction& instruction);
    std::string handleUnaryOp(const IRInstruction& instruction);
    std::string handleJump(const IRInstruction& instruction);
    std::string handleJumpIfFalse(const IRInstruction& instruction);
    std::string handleJumpIfTrue(const IRInstruction& instruction);
    std::string handleCall(const IRInstruction& instruction);
    std::string handleReturn(const IRInstruction& instruction);
    std::string handlePrint(const IRInstruction& instruction);
    std::string handleInput(const IRInstruction& instruction);
    std::string handleArrayNew(const IRInstruction& instruction);
    std::string handleArrayGet(const IRInstruction& instruction);
    std::string handleArraySet(const IRInstruction& instruction);
    std::string handleMemberGet(const IRInstruction& instruction);
    std::string handleConvert(const IRInstruction& instruction);
    std::string handleLabel(const IRInstruction& instruction);
    std::string handleNop(const IRInstruction& instruction);
    
    // Utility methods
    std::string getIndent(int level) const;
    std::string getPythonOperator(TokenType op) const;
};

} // namespace vypr

#endif // VYPR_CODE_GENERATOR_H 