#include "lexer.h"
#include "exceptions.h"
#include <cctype>
#include <stdexcept>
#include <queue>

namespace vypr {

std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"var", TokenType::VAR},
    {"func", TokenType::FUNC},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"loop", TokenType::LOOP},
    {"in", TokenType::IN},
    {"times", TokenType::TIMES},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT},
    {"true", TokenType::BOOLEAN},
    {"false", TokenType::BOOLEAN}
};

Lexer::Lexer(const std::string& source)
    : source(source), position(0), line(1), column(1), current_char('\0'), current_indent(0), at_line_start(true) {
    
    if (!source.empty()) {
        current_char = source[position];
    }
    indent_stack.push(0);  // Start with 0 indentation
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = next_token();
    
    while (token.type != TokenType::EOF_TOKEN) {
        tokens.push_back(token);
        token = next_token();
    }
    
    // Add final EOF token
    tokens.push_back(token);
    
    return tokens;
}

void Lexer::advance() {
    if (position < source.length()) {
        if (current_char == '\n') {
            line++;
            column = 0;
        }
        position++;
        if (position >= source.length()) {
            current_char = '\0'; // End of source
        } else {
            current_char = source[position];
            column++;
        }
    } else {
        current_char = '\0'; // Already at end of source
    }
}

char Lexer::peek() const {
    if (position + 1 >= source.length()) {
        return '\0';
    }
    return source[position + 1];
}

Token Lexer::next_token() {
    // Check token queue first
    if (!token_queue.empty()) {
        Token token = token_queue.front();
        token_queue.pop();
        return token;
    }

    while (current_char != '\0') {
        // Handle whitespace at the start of a line for indentation
        if (at_line_start) {
            handle_newline(); // Processes indentation and consumes initial whitespace/newline
            at_line_start = false; // No longer at the start
            // If handle_newline generated tokens, return the first one
            if (!token_queue.empty()) {
                Token token = token_queue.front();
                token_queue.pop();
                return token;
            }
            // If no tokens generated (e.g., just whitespace consumed), continue loop
            if (current_char == '\0') break; // EOF reached during handle_newline
            continue;
        }
        
        // Skip non-newline whitespace in the middle of a line
        if (std::isspace(current_char) && current_char != '\n') {
            skip_whitespace(); 
            continue; 
        }

        // If we encounter a newline *after* the start of the line, mark the next char as start
        if (current_char == '\n') {
            int prev_line = line;
            int prev_col = column;
            advance(); // Consume the newline
            at_line_start = true; // Next token will be at line start
            return Token(TokenType::NEWLINE, prev_line, prev_col); // Return NEWLINE token
        }
        
        // Check for comments (after handling whitespace)
        if (current_char == '/' && peek() == '/') {
            // Skip the comment line
            while (current_char != '\n' && current_char != '\0') {
                advance();
            }
             // Don't set at_line_start here, the loop will handle the \n or EOF
            continue; 
        }

        // Reset at_line_start flag since we are processing a non-whitespace/comment char
        at_line_start = false; 

        // Handle actual tokens (Identifiers, Numbers, Strings, Operators)
        if (is_alpha(current_char) || current_char == '_') {
            return handle_identifier();
        }
        
        if (is_digit(current_char)) {
            return handle_number();
        }
        
        if (current_char == '\"' || current_char == '\'') {
            return handle_string();
        }

        // Handle operators etc.
        return handle_operator();
    }

    // End of file: Emit remaining DEDENT tokens
    while (indent_stack.size() > 1) {
        indent_stack.pop();
        // Line/column for EOF dedents might be approximate
        token_queue.push(Token(TokenType::DEDENT, line, column)); 
    }
    // Return queued DEDENT or EOF
    if (!token_queue.empty()) {
        Token token = token_queue.front();
        token_queue.pop();
        return token;
    }
    
    return Token(TokenType::EOF_TOKEN, line, column);
}

void Lexer::skip_whitespace() {
    while (std::isspace(current_char) && current_char != '\n') {
        advance();
    }
}

void Lexer::handle_newline() {
    // Consumes newline if present, then counts indentation
    if (current_char == '\n') {
         advance(); // Consume the actual newline char
    }

    // Skip any immediately following empty lines (only whitespace/comments)
    while (current_char != '\0') {
        int start_pos = position;
        skip_whitespace(); // Skip leading spaces/tabs on the potential new line
        
        // Check for comment line
        if (current_char == '/' && peek() == '/') {
            while (current_char != '\n' && current_char != '\0') {
                advance();
            }
            if (current_char == '\n') {
                 advance(); // Consume newline after comment
                 continue; // Look for next non-empty line
            } else {
                 break; // EOF after comment
            }
        } 
        // Check for empty line (only newline follows whitespace)
        else if (current_char == '\n') {
             advance(); // Consume the newline of the empty line
             continue; // Look for next non-empty line
        } 
        // Found non-whitespace/comment/newline character or EOF
        else { 
            position = start_pos; // Backtrack to count indent from start
            current_char = source[position];
            column = 1; // Reset column for indent count
            break;
        }
    }

    // We are now at the first non-whitespace character of the line or EOF
    if (current_char != '\0') {
        int indent_level = count_indent();
        process_indent(indent_level); // This should queue INDENT/DEDENT tokens
        // The main loop should check this queue first.
    }
}

int Lexer::count_indent() {
    int indent_level = 0;
    
    // Count spaces
    while (current_char == ' ') {
        indent_level++;
        advance();
    }
    
    // Count tabs (4 spaces per tab)
    while (current_char == '\t') {
        indent_level += 4;
        advance();
    }
    
    return indent_level;
}

void Lexer::process_indent(int indent_level) {
    int prev_indent = indent_stack.top();
    current_indent = indent_level; // Update current indent level

    if (indent_level > prev_indent) {
        indent_stack.push(indent_level);
        token_queue.push(Token(TokenType::INDENT, line, 1));
    } else if (indent_level < prev_indent) {
        while (!indent_stack.empty() && indent_stack.top() > indent_level) {
            indent_stack.pop();
            token_queue.push(Token(TokenType::DEDENT, line, 1));
        }
        if (indent_stack.empty() || indent_stack.top() != indent_level) {
            throw LexerError("Invalid indentation at line " + std::to_string(line)); 
        }
    } 
}

Token Lexer::handle_identifier() {
    std::string identifier;
    
    while (is_alphanumeric(current_char)) {
        identifier += current_char;
        advance();
    }
    
    // Check if it's a keyword
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        if (it->second == TokenType::BOOLEAN) {
            bool value = (identifier == "true");
            return Token(TokenType::BOOLEAN, value, line, column - identifier.length());
        }
        return Token(it->second, line, column - identifier.length());
    }
    
    // It's an identifier
    return Token(TokenType::IDENTIFIER, identifier, line, column - identifier.length());
}

Token Lexer::handle_number() {
    std::string num_str;
    bool is_float = false;
    
    // Collect digits
    while (is_digit(current_char) || current_char == '.') {
        if (current_char == '.') {
            if (is_float) {
                // Second decimal point is invalid
                throw std::runtime_error("Invalid number format at line " + std::to_string(line));
            }
            is_float = true;
        }
        
        num_str += current_char;
        advance();
    }
    
    // Convert to number
    if (is_float) {
        double value = std::stod(num_str);
        return Token(TokenType::FLOAT, value, line, column - num_str.length());
    } else {
        int value = std::stoi(num_str);
        return Token(TokenType::INTEGER, value, line, column - num_str.length());
    }
}

Token Lexer::handle_string() {
    char quote = current_char;
    advance();  // Skip the opening quote
    
    std::string value;
    while (current_char != '\0' && current_char != quote) {
        // Handle escape sequences
        if (current_char == '\\' && peek() == quote) {
            advance();  // Skip the backslash
            value += quote;  // Add the quote
            advance();  // Skip the quote
            continue;
        }
        
        value += current_char;
        advance();
    }
    
    if (current_char != quote) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(line));
    }
    
    advance();  // Skip the closing quote
    
    // Skip any whitespace after the string
    while (std::isspace(current_char) && current_char != '\n') {
        advance();
    }
    
    return Token(TokenType::STRING, value, line, column - value.length() - 2);
}

Token Lexer::handle_operator() {
    switch (current_char) {
        case '+':
            advance();
            return Token(TokenType::PLUS, line, column - 1);
        
        case '-':
            advance();
            return Token(TokenType::MINUS, line, column - 1);
        
        case '*':
            advance();
            return Token(TokenType::MULTIPLY, line, column - 1);
        
        case '/':
            advance();
            return Token(TokenType::DIVIDE, line, column - 1);
        
        case '%':
            advance();
            return Token(TokenType::MODULO, line, column - 1);
        
        case '^':
            advance();
            return Token(TokenType::CONCAT, line, column - 1);
        
        case '=':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::EQUAL, line, column - 2);
            }
            return Token(TokenType::ASSIGN, line, column - 1);
        
        case '!':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, line, column - 2);
            }
            return Token(TokenType::NOT, line, column - 1);
        
        case '>':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, line, column - 2);
            }
            return Token(TokenType::GREATER, line, column - 1);
        
        case '<':
            advance();
            if (current_char == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, line, column - 2);
            }
            return Token(TokenType::LESS, line, column - 1);
        
        case '&':
            advance();
            if (current_char == '&') {
                advance();
                return Token(TokenType::AND, line, column - 2);
            }
            throw std::runtime_error("Unexpected character '&' at line " + std::to_string(line));
        
        case '|':
            advance();
            if (current_char == '|') {
                advance();
                return Token(TokenType::OR, line, column - 2);
            }
            throw std::runtime_error("Unexpected character '|' at line " + std::to_string(line));
        
        case '(':
            advance();
            return Token(TokenType::LPAREN, line, column - 1);
        
        case ')':
            advance();
            return Token(TokenType::RPAREN, line, column - 1);
        
        case '[':
            advance();
            return Token(TokenType::LBRACKET, line, column - 1);
        
        case ']':
            advance();
            return Token(TokenType::RBRACKET, line, column - 1);
        
        case ':':
            advance();
            return Token(TokenType::COLON, line, column - 1);
        
        case ',':
            advance();
            return Token(TokenType::COMMA, line, column - 1);
        
        case '.':
            advance();
            return Token(TokenType::DOT, line, column - 1);
        
        default:
            char c = current_char;
            advance();
            throw std::runtime_error("Unexpected character '" + std::string(1, c) + "' at line " + std::to_string(line));
    }
}

bool Lexer::is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(c);
}

bool Lexer::is_alphanumeric(char c) const {
    return is_alpha(c) || is_digit(c);
}

} // namespace vypr 