#include "ir_generator.h"
#include <sstream>
#include <variant>

namespace vypr {

IRGenerator::IRGenerator() : currentFunction(nullptr), tempCounter(0) {
    // Create the main function
    functions.push_back(IRFunction("__main__", {}));
    currentFunction = &functions.back();
}

std::vector<IRFunction> IRGenerator::generate(const std::shared_ptr<Program>& program) {
    visit(program);
    return functions;
}

void IRGenerator::enterFunction(const std::string& name, const std::vector<std::string>& parameters) {
    functions.push_back(IRFunction(name, parameters));
    currentFunction = &functions.back();
    tempCounter = 0;
    variables.clear();
}

void IRGenerator::exitFunction() {
    // Main function is always the first one
    currentFunction = &functions.front();
}

std::string IRGenerator::generateTemp() {
    return "t" + std::to_string(tempCounter++);
}

void IRGenerator::emit(const IRInstruction& instruction) {
    currentFunction->instructions.push_back(instruction);
}

void IRGenerator::visit(const std::shared_ptr<Program>& program) {
    for (const auto& stmt : program->statements) {
        visit(stmt);
    }
}

void IRGenerator::visit(const StatementPtr& stmt) {
    if (auto varDecl = std::dynamic_pointer_cast<VarDeclarationStatement>(stmt)) {
        visitVarDeclaration(varDecl);
    } else if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclaration>(stmt)) {
        visitFunctionDeclaration(funcDecl);
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        visitExpressionStatement(exprStmt);
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        visitIfStatement(ifStmt);
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
        visitWhileStatement(whileStmt);
    } else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        visitReturnStatement(returnStmt);
    } else if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        visitBlockStatement(blockStmt);
    } else if (auto printStmt = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        visitPrintStatement(printStmt);
    } else if (auto inputStmt = std::dynamic_pointer_cast<InputStatement>(stmt)) {
        visitInputStatement(inputStmt);
    } else if (auto loopInStmt = std::dynamic_pointer_cast<LoopInStatement>(stmt)) {
        visitLoopInStatement(loopInStmt);
    } else if (auto loopTimesStmt = std::dynamic_pointer_cast<LoopTimesStatement>(stmt)) {
        visitLoopTimesStatement(loopTimesStmt);
    }
}

std::string IRGenerator::visit(const ExpressionPtr& expr) {
    if (auto binary = std::dynamic_pointer_cast<BinaryExpression>(expr)) {
        return visitBinaryExpression(binary);
    } else if (auto unary = std::dynamic_pointer_cast<UnaryExpression>(expr)) {
        return visitUnaryExpression(unary);
    } else if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(expr)) {
        return visitLiteralExpression(literal);
    } else if (auto variable = std::dynamic_pointer_cast<VariableExpression>(expr)) {
        return visitVariableExpression(variable);
    } else if (auto call = std::dynamic_pointer_cast<CallExpression>(expr)) {
        return visitCallExpression(call);
    } else if (auto array = std::dynamic_pointer_cast<ArrayExpression>(expr)) {
        return visitArrayExpression(array);
    } else if (auto arrayAccess = std::dynamic_pointer_cast<ArrayAccessExpression>(expr)) {
        return visitArrayAccessExpression(arrayAccess);
    } else if (auto memberAccess = std::dynamic_pointer_cast<MemberAccessExpression>(expr)) {
        return visitMemberAccessExpression(memberAccess);
    }
    
    // Fallback
    return generateTemp();
}

void IRGenerator::visitVarDeclaration(const std::shared_ptr<VarDeclarationStatement>& stmt) {
    std::string temp;
    
    if (stmt->initializer != nullptr) {
        temp = visit(stmt->initializer);
        emit(IRInstruction(IROpCode::STORE_VAR, {stmt->name, temp}));
    }
    
    // Track the variable
    variables[stmt->name] = 1;
}

void IRGenerator::visitFunctionDeclaration(const std::shared_ptr<FunctionDeclaration>& stmt) {
    // Save current function
    IRFunction* previousFunction = currentFunction;
    
    // Create new function
    enterFunction(stmt->name, stmt->parameters);
    
    // Add parameters to variables
    for (const auto& param : stmt->parameters) {
        variables[param] = 1;
    }
    
    // Generate IR for function body
    visit(stmt->body);
    
    // Add implicit return if needed
    if (currentFunction->instructions.empty() || 
        currentFunction->instructions.back().opcode != IROpCode::RETURN) {
        emit(IRInstruction(IROpCode::RETURN));
    }
    
    // Restore previous function
    currentFunction = previousFunction;
}

void IRGenerator::visitExpressionStatement(const std::shared_ptr<ExpressionStatement>& stmt) {
    visit(stmt->expression);
}

void IRGenerator::visitIfStatement(const std::shared_ptr<IfStatement>& stmt) {
    std::string condition = visit(stmt->condition);
    
    std::string elseLabel = currentFunction->generateLabel();
    std::string endLabel = currentFunction->generateLabel();
    
    // Jump to else branch if condition is false
    emit(IRInstruction(IROpCode::JUMP_IF_FALSE, {condition, elseLabel}));
    
    // Then branch
    visit(stmt->then_branch);
    
    // Jump to end
    emit(IRInstruction(IROpCode::JUMP, {endLabel}));
    
    // Else branch
    emit(IRInstruction(IROpCode::LABEL, {elseLabel}));
    
    if (stmt->else_branch != nullptr) {
        visit(stmt->else_branch);
    }
    
    // End label
    emit(IRInstruction(IROpCode::LABEL, {endLabel}));
}

void IRGenerator::visitWhileStatement(const std::shared_ptr<WhileStatement>& stmt) {
    std::string loopLabel = currentFunction->generateLabel();
    std::string endLabel = currentFunction->generateLabel();
    
    // Loop label
    emit(IRInstruction(IROpCode::LABEL, {loopLabel}));
    
    // Evaluate condition
    std::string condition = visit(stmt->condition);
    
    // Jump to end if condition is false
    emit(IRInstruction(IROpCode::JUMP_IF_FALSE, {condition, endLabel}));
    
    // Loop body
    visit(stmt->body);
    
    // Jump back to start
    emit(IRInstruction(IROpCode::JUMP, {loopLabel}));
    
    // End label
    emit(IRInstruction(IROpCode::LABEL, {endLabel}));
}

void IRGenerator::visitReturnStatement(const std::shared_ptr<ReturnStatement>& stmt) {
    if (stmt->value != nullptr) {
        std::string value = visit(stmt->value);
        emit(IRInstruction(IROpCode::RETURN, {value}));
    } else {
        emit(IRInstruction(IROpCode::RETURN));
    }
}

void IRGenerator::visitBlockStatement(const std::shared_ptr<BlockStatement>& stmt) {
    for (const auto& s : stmt->statements) {
        visit(s);
    }
}

void IRGenerator::visitPrintStatement(const std::shared_ptr<PrintStatement>& stmt) {
    std::string value = visit(stmt->expression);
    emit(IRInstruction(IROpCode::PRINT, {value}));
}

void IRGenerator::visitInputStatement(const std::shared_ptr<InputStatement>& stmt) {
    emit(IRInstruction(IROpCode::INPUT, {stmt->variable}));
    
    // Track the variable
    variables[stmt->variable] = 1;
}

void IRGenerator::visitLoopInStatement(const std::shared_ptr<LoopInStatement>& stmt) {
    std::string iterable = visit(stmt->iterable);
    
    std::string indexVar = generateTemp();
    std::string loopLabel = currentFunction->generateLabel();
    std::string endLabel = currentFunction->generateLabel();
    
    // Initialize index to 0
    emit(IRInstruction(IROpCode::LOAD_CONST, {indexVar, "0"}));
    
    // Loop label
    emit(IRInstruction(IROpCode::LABEL, {loopLabel}));
    
    // Check if index < array length
    std::string lengthTemp = generateTemp();
    std::string conditionTemp = generateTemp();
    emit(IRInstruction(IROpCode::MEMBER_GET, {lengthTemp, iterable, "length"}));
    emit(IRInstruction(IROpCode::BINARY_OP, {conditionTemp, indexVar, "<", lengthTemp}));
    
    // Jump to end if condition is false
    emit(IRInstruction(IROpCode::JUMP_IF_FALSE, {conditionTemp, endLabel}));
    
    // Get current item
    std::string itemTemp = generateTemp();
    emit(IRInstruction(IROpCode::ARRAY_GET, {itemTemp, iterable, indexVar}));
    
    // Store in loop variable
    emit(IRInstruction(IROpCode::STORE_VAR, {stmt->variable, itemTemp}));
    
    // Loop body
    visit(stmt->body);
    
    // Increment index
    std::string nextIndexTemp = generateTemp();
    emit(IRInstruction(IROpCode::BINARY_OP, {nextIndexTemp, indexVar, "+", "1"}));
    emit(IRInstruction(IROpCode::STORE_VAR, {indexVar, nextIndexTemp}));
    
    // Jump back to start
    emit(IRInstruction(IROpCode::JUMP, {loopLabel}));
    
    // End label
    emit(IRInstruction(IROpCode::LABEL, {endLabel}));
}

void IRGenerator::visitLoopTimesStatement(const std::shared_ptr<LoopTimesStatement>& stmt) {
    std::string count = visit(stmt->count);
    
    std::string indexVar = generateTemp();
    std::string loopLabel = currentFunction->generateLabel();
    std::string endLabel = currentFunction->generateLabel();
    
    // Initialize index to 0
    emit(IRInstruction(IROpCode::LOAD_CONST, {indexVar, "0"}));
    
    // Loop label
    emit(IRInstruction(IROpCode::LABEL, {loopLabel}));
    
    // Check if index < count
    std::string conditionTemp = generateTemp();
    emit(IRInstruction(IROpCode::BINARY_OP, {conditionTemp, indexVar, "<", count}));
    
    // Jump to end if condition is false
    emit(IRInstruction(IROpCode::JUMP_IF_FALSE, {conditionTemp, endLabel}));
    
    // Loop body
    visit(stmt->body);
    
    // Increment index
    std::string nextIndexTemp = generateTemp();
    emit(IRInstruction(IROpCode::BINARY_OP, {nextIndexTemp, indexVar, "+", "1"}));
    emit(IRInstruction(IROpCode::STORE_VAR, {indexVar, nextIndexTemp}));
    
    // Jump back to start
    emit(IRInstruction(IROpCode::JUMP, {loopLabel}));
    
    // End label
    emit(IRInstruction(IROpCode::LABEL, {endLabel}));
}

std::string IRGenerator::visitBinaryExpression(const std::shared_ptr<BinaryExpression>& expr) {
    std::string left = visit(expr->left);
    std::string right = visit(expr->right);
    std::string result = generateTemp();
    
    if (expr->op == TokenType::ASSIGN) {
        // Handle assignment
        if (auto var = std::dynamic_pointer_cast<VariableExpression>(expr->left)) {
            emit(IRInstruction(IROpCode::STORE_VAR, {var->name, right}));
            return right;
        } else if (auto arrayAccess = std::dynamic_pointer_cast<ArrayAccessExpression>(expr->left)) {
            std::string array = visit(arrayAccess->array);
            std::string index = visit(arrayAccess->index);
            emit(IRInstruction(IROpCode::ARRAY_SET, {array, index, right}));
            return right;
        }
    }
    
    // Convert token type to operator string
    std::string op;
    switch (expr->op) {
        case TokenType::PLUS: op = "+"; break;
        case TokenType::MINUS: op = "-"; break;
        case TokenType::MULTIPLY: op = "*"; break;
        case TokenType::DIVIDE: op = "/"; break;
        case TokenType::CONCAT: op = "^"; break;
        case TokenType::EQUAL: op = "=="; break;
        case TokenType::NOT_EQUAL: op = "!="; break;
        case TokenType::LESS: op = "<"; break;
        case TokenType::LESS_EQUAL: op = "<="; break;
        case TokenType::GREATER: op = ">"; break;
        case TokenType::GREATER_EQUAL: op = ">="; break;
        case TokenType::AND: op = "&&"; break;
        case TokenType::OR: op = "||"; break;
        default: op = "?"; break;
    }
    
    emit(IRInstruction(IROpCode::BINARY_OP, {result, left, op, right}));
    return result;
}

std::string IRGenerator::visitUnaryExpression(const std::shared_ptr<UnaryExpression>& expr) {
    std::string operand = visit(expr->right);
    std::string result = generateTemp();
    
    // Convert token type to operator string
    std::string op;
    switch (expr->op) {
        case TokenType::MINUS: op = "-"; break;
        case TokenType::NOT: op = "!"; break;
        default: op = "?"; break;
    }
    
    emit(IRInstruction(IROpCode::UNARY_OP, {result, op, operand}));
    return result;
}

std::string IRGenerator::visitLiteralExpression(const std::shared_ptr<LiteralExpression>& expr) {
    std::string result = generateTemp();
    std::string value;
    
    // Convert the literal value to a string
    if (std::holds_alternative<int>(expr->value)) {
        value = std::to_string(std::get<int>(expr->value));
    } else if (std::holds_alternative<double>(expr->value)) {
        value = std::to_string(std::get<double>(expr->value));
    } else if (std::holds_alternative<bool>(expr->value)) {
        value = std::get<bool>(expr->value) ? "true" : "false";
    } else if (std::holds_alternative<std::string>(expr->value)) {
        value = "\"" + std::get<std::string>(expr->value) + "\"";
    }
    
    emit(IRInstruction(IROpCode::LOAD_CONST, {result, value}));
    return result;
}

std::string IRGenerator::visitVariableExpression(const std::shared_ptr<VariableExpression>& expr) {
    std::string result = generateTemp();
    emit(IRInstruction(IROpCode::LOAD_VAR, {result, expr->name}));
    return result;
}

std::string IRGenerator::visitCallExpression(const std::shared_ptr<CallExpression>& expr) {
    std::string callee_name = expr->callee;
    std::vector<std::string> argValues;

    // Evaluate arguments
    for (const auto& arg : expr->arguments) {
        argValues.push_back(visit(arg));
    }

    // Check for built-in type conversion functions
    if ((callee_name == "int" || callee_name == "float" || callee_name == "str" || callee_name == "bool") && argValues.size() == 1) {
        std::string result = generateTemp();
        // Emit CONVERT instruction: result = type(source)
        emit(IRInstruction(IROpCode::CONVERT, {result, callee_name, argValues[0]}));
        return result;
    }

    // Handle regular function calls
    std::string argsStr;
    for (size_t i = 0; i < argValues.size(); ++i) {
        if (i > 0) {
            argsStr += ", ";
        }
        argsStr += argValues[i];
    }
    
    std::string result = generateTemp();
    emit(IRInstruction(IROpCode::CALL, {result, callee_name, argsStr}));
    return result;
}

std::string IRGenerator::visitArrayExpression(const std::shared_ptr<ArrayExpression>& expr) {
    std::vector<std::string> elementValues;
    
    // Evaluate elements
    for (const auto& element : expr->elements) {
        elementValues.push_back(visit(element));
    }
    
    // Join element values with commas
    std::string elementsStr;
    for (size_t i = 0; i < elementValues.size(); ++i) {
        if (i > 0) {
            elementsStr += ", ";
        }
        elementsStr += elementValues[i];
    }
    
    std::string result = generateTemp();
    emit(IRInstruction(IROpCode::ARRAY_NEW, {result, elementsStr}));
    return result;
}

std::string IRGenerator::visitArrayAccessExpression(const std::shared_ptr<ArrayAccessExpression>& expr) {
    std::string array = visit(expr->array);
    std::string index = visit(expr->index);
    std::string result = generateTemp();
    
    emit(IRInstruction(IROpCode::ARRAY_GET, {result, array, index}));
    return result;
}

std::string IRGenerator::visitMemberAccessExpression(const std::shared_ptr<MemberAccessExpression>& expr) {
    std::string object = visit(expr->object);
    std::string result = generateTemp();
    
    emit(IRInstruction(IROpCode::MEMBER_GET, {result, object, expr->member}));
    return result;
}

std::string irOpCodeToString(IROpCode opcode) {
    switch (opcode) {
        case IROpCode::LOAD_CONST: return "LOAD_CONST";
        case IROpCode::LOAD_VAR: return "LOAD_VAR";
        case IROpCode::STORE_VAR: return "STORE_VAR";
        case IROpCode::BINARY_OP: return "BINARY_OP";
        case IROpCode::UNARY_OP: return "UNARY_OP";
        case IROpCode::JUMP: return "JUMP";
        case IROpCode::JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case IROpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case IROpCode::CALL: return "CALL";
        case IROpCode::RETURN: return "RETURN";
        case IROpCode::PRINT: return "PRINT";
        case IROpCode::INPUT: return "INPUT";
        case IROpCode::ARRAY_NEW: return "ARRAY_NEW";
        case IROpCode::ARRAY_GET: return "ARRAY_GET";
        case IROpCode::ARRAY_SET: return "ARRAY_SET";
        case IROpCode::MEMBER_GET: return "MEMBER_GET";
        case IROpCode::LABEL: return "LABEL";
        case IROpCode::CONVERT: return "CONVERT";
        case IROpCode::NOP: return "NOP";
        default: return "UNKNOWN";
    }
}

} // namespace vypr 