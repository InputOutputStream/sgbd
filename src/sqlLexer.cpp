#include "../include/sqlLexer.hpp"
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <ostream>
#include <fstream>
#include <cstdlib>
#include <unordered_map>
#include <cctype>

Lexer::Lexer(const std::string& content) : content(content), position(0) {
    if (content.empty()) {
        current_char = '\0';
        return;
    }
    
    current_char = content[0];
    
    // Initialize special characters for lexing
    special_chars = {'=', ';', ',','\'' , '(', ')', '{', '}', '[', ']', '"'};
}

std::string Lexer::read_file(const std::string &path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        return "";
    }
    
    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    return content;
}

void Lexer::advance() {
    if (position < content.size() - 1) {
        position++;
        current_char = content[position];
    } else {
        current_char = '\0';
    }
}

void Lexer::skip_whitespace() {
    while (std::isspace(current_char) && current_char != '\0') {
        advance();
    }
}

char Lexer::peek_next() const {
    if (position + 1 < content.size()) {
        return content[position + 1];
    }
    return '\0';
}

bool Lexer::is_at_end() const{
    return position >= content.size() || current_char == '\0';
}

std::unique_ptr<Token> Lexer::get_next_token() {
    while (current_char != '\0' && position < content.size()) {
        if (std::isspace(current_char)) {
            skip_whitespace();
            continue;
        }

        if (std::isalpha(current_char) || current_char == '_') {
            /**
             * this is suppose to collect all tokens different from white spaces whether sql clauses or table names, it will be to the parser to decide
             * what to do with them.
             */
            return collect_id();
        }
        
        if (std::isdigit(current_char)) {
            return collect_number();
        }
        
        if (current_char == '"') {
            return collect_string();
        }

        if(is_at_end())
        {
            return std::make_unique<Token>(TokenType::END_FILE);
        }

        switch (current_char) {
            case '=':
                return advance_with_token(TokenType::EQUALS, "=");
            case ',':
                return advance_with_token(TokenType::COMMA, ",");
            case '*':
                return advance_with_token(TokenType::STAR, "*");
            case '\'':
                return advance_with_token(TokenType::QUOTE, "'");
            case ';':
                return advance_with_token(TokenType::SEMI, ";");
            case '(':
                return advance_with_token(TokenType::LPAREN, "(");
            case ')':
                return advance_with_token(TokenType::RPAREN, ")");
            case '{':
                return advance_with_token(TokenType::LBRACE, "{");
            case '}':
                return advance_with_token(TokenType::RBRACE, "}");
            case '[':
                return advance_with_token(TokenType::LSBRACE, "[");
            case ']':
                return advance_with_token(TokenType::RSBRACE, "]");

            // Handle comparison operators
            case '<':
                if (peek_next() == '=') {
                    advance(); // consume '='       
                    return advance_with_token(TokenType::LESS_EQUAL, "<=");
                } else if (peek_next() == '>') {
                    advance(); // consume '>'       
                    return advance_with_token(TokenType::NOT_EQUAL, "<>");
                } else {
                    return advance_with_token(TokenType::LESS_THAN, "<");       
                }
            case '>':   
                if (peek_next() == '=') {
                    advance(); // consume '='       
                    return advance_with_token(TokenType::GREATER_EQUAL, ">=");
                } else {
                    return advance_with_token(TokenType::GREATER_THAN, ">");       
                }
            case '!':
                if (peek_next() == '=') {
                    advance(); // consume '='       
                    return advance_with_token(TokenType::NOT_EQUAL, "!=");
                } else {
                    std::cerr << "ERROR: Unexpected character '!' at position " << position << "\n";
                    advance();
                    return nullptr; // Invalid token
                }
            default:
                advance();
                continue;
        }
    }

    return std::make_unique<Token>(TokenType::END_FILE, "");
}

std::unique_ptr<Token> Lexer::collect_string() {
    advance(); // Skip opening quote
    std::string value;

    while (current_char != '"' && current_char != '\0') {
        value += current_char;
        advance();
    }
    
    if (current_char != '"') {
        std::cerr << "ERROR: Unterminated string literal!\n";
        return nullptr;
    }
    
    advance(); // Skip closing quote
    return std::make_unique<Token>(TokenType::STRING, value);
}

std::unique_ptr<Token> Lexer::collect_id() {
    std::string value;

    while (std::isalnum(current_char) || current_char == '_') {
        value += current_char;
        advance();
    }
    
    return std::make_unique<Token>(TokenType::ID, value);
}

std::unique_ptr<Token> Lexer::collect_number() {
    std::string value;

    while (std::isdigit(current_char)) {
        value += current_char;
        advance();
    }

    return std::make_unique<Token>(TokenType::NUMBER, value);
}

std::unique_ptr<Token> Lexer::advance_with_token(TokenType t, std::string value) {
    advance();
    return std::make_unique<Token>(t, std::move(value));
}