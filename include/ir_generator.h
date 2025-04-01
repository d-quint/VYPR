#ifndef VYPR_IR_GENERATOR_H
#define VYPR_IR_GENERATOR_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ast.h"
#include <sstream>

namespace vypr {

// Forward declaration
class IRInstruction;

// IR instruction types
enum class IROpCode {
    LOAD_CONST,        // Push a constant value onto the stack
    LOAD_VAR,          // Load variable value
    STORE_VAR,         // Store value to variable
    BINARY_OP,         // Binary operation
    UNARY_OP,          // Unary operation
    JUMP,              // Unconditional jump
    JUMP_IF_FALSE,     // Jump if condition is false
    JUMP_IF_TRUE,      // Jump if condition is true
    CALL,              // Function call
    RETURN,            // Return from function
    PRINT,             // Print value
    INPUT,             // Get input
    ARRAY_NEW,         // Create new array
    ARRAY_GET,         // Get element from array
    ARRAY_SET,         // Set element in array
    MEMBER_GET,        // Get object member
    LABEL,             // Label for jumps
    NOP                // No operation
};

// Helper function to convert IR opcode to string
std::string irOpCodeToString(IROpCode opcode);

// Basic IR instruction
struct IRInstruction {
    IROpCode opcode;
    std::vector<std::string> operands;
    
    IRInstruction(IROpCode opcode, const std::vector<std::string>& operands = {})
        : opcode(opcode), operands(operands) {}
    
    std::string toString() const {
        std::stringstream ss;
        ss << irOpCodeToString(opcode);
        
        if (!operands.empty()) {
            ss << " ";
            for (size_t i = 0; i < operands.size(); ++i) {
                ss << operands[i];
                if (i < operands.size() - 1) {
                    ss << ", ";
                }
            }
        }
        
        return ss.str();
    }
};

// Function information for IR generation
struct IRFunction {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<IRInstruction> instructions;
    int labelCounter;
    
    IRFunction(std::string name, std::vector<std::string> parameters)
        : name(std::move(name)), parameters(std::move(parameters)), labelCounter(0) {}
    
    std::string generateLabel() {
        return "L" + std::to_string(labelCounter++);
    }
};

class IRGenerator {
public:
    IRGenerator();
    std::vector<IRFunction> generate(const std::shared_ptr<Program>& program);
    
private:
    std::vector<IRFunction> functions;
    IRFunction* currentFunction;
    std::unordered_map<std::string, int> variables;
    int tempCounter;
    
    // Helper methods
    void enterFunction(const std::string& name, const std::vector<std::string>& parameters);
    void exitFunction();
    std::string generateTemp();
    void emit(const IRInstruction& instruction);
    
    // Visitor methods for AST nodes
    void visit(const std::shared_ptr<Program>& program);
    void visit(const StatementPtr& stmt);
    std::string visit(const ExpressionPtr& expr);
    
    void visitVarDeclaration(const std::shared_ptr<VarDeclarationStatement>& stmt);
    void visitFunctionDeclaration(const std::shared_ptr<FunctionDeclaration>& stmt);
    void visitExpressionStatement(const std::shared_ptr<ExpressionStatement>& stmt);
    void visitIfStatement(const std::shared_ptr<IfStatement>& stmt);
    void visitWhileStatement(const std::shared_ptr<WhileStatement>& stmt);
    void visitReturnStatement(const std::shared_ptr<ReturnStatement>& stmt);
    void visitBlockStatement(const std::shared_ptr<BlockStatement>& stmt);
    void visitPrintStatement(const std::shared_ptr<PrintStatement>& stmt);
    void visitInputStatement(const std::shared_ptr<InputStatement>& stmt);
    void visitLoopInStatement(const std::shared_ptr<LoopInStatement>& stmt);
    void visitLoopTimesStatement(const std::shared_ptr<LoopTimesStatement>& stmt);
    
    std::string visitBinaryExpression(const std::shared_ptr<BinaryExpression>& expr);
    std::string visitUnaryExpression(const std::shared_ptr<UnaryExpression>& expr);
    std::string visitLiteralExpression(const std::shared_ptr<LiteralExpression>& expr);
    std::string visitVariableExpression(const std::shared_ptr<VariableExpression>& expr);
    std::string visitCallExpression(const std::shared_ptr<CallExpression>& expr);
    std::string visitArrayExpression(const std::shared_ptr<ArrayExpression>& expr);
    std::string visitArrayAccessExpression(const std::shared_ptr<ArrayAccessExpression>& expr);
    std::string visitMemberAccessExpression(const std::shared_ptr<MemberAccessExpression>& expr);
};

} // namespace vypr

#endif // VYPR_IR_GENERATOR_H 