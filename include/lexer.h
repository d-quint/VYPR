#ifndef VYPR_LEXER_H
#define VYPR_LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <queue>
#include "token.h"

namespace vypr {

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    Token next_token();

private:
    std::string source;
    int position;
    int line;
    int column;
    char current_char;
    int current_indent;
    std::stack<int> indent_stack;
    bool at_line_start;
    std::queue<Token> token_queue;
    
    static std::unordered_map<std::string, TokenType> keywords;

    void advance();
    char peek() const;
    void skip_whitespace();
    void handle_newline();
    int count_indent();
    void process_indent(int indent_level);
    
    Token handle_identifier();
    Token handle_number();
    Token handle_string();
    Token handle_operator();
    
    bool is_alpha(char c) const;
    bool is_digit(char c) const;
    bool is_alphanumeric(char c) const;
};

} // namespace vypr

#endif // VYPR_LEXER_H 