#ifndef SQL_LEXER_H
#define SQL_LEXER_H

#include "definitions.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

enum class TokenType {
    ID,
    EQUALS,
    STRING,
    SEMI,
    COMMA,
    LPAREN,
    RPAREN,
    LBRACE,
    NUMBER,
    RBRACE,
    LSBRACE,
    RSBRACE,
    STAR,
    QUOTE,
    END_FILE
};

struct Token {
    TokenType type;
    std::string value; 
    
    Token(TokenType t, std::string v = "") : type(t), value(std::move(v)) {}
};

class Lexer {
    private:
        std::string content;
        std::unordered_set<char> special_chars;  
        size_t position = 0;  
        char current_char = '\0';
        
        // Private helper methods
        std::unique_ptr<Token> advance_with_token(TokenType type, std::string value = "");
        void advance();
        void skip_whitespace();
        char peek_next() const;
    
    public:
        // Constructors
        Lexer() = default;
        explicit Lexer(const std::string& content);
        
        std::unique_ptr<Token> get_next_token();
        bool is_at_end() const;
        
        static std::string read_file(const std::string& path);  // Return string, take const ref
        
    private:
        std::unique_ptr<Token> collect_string();
        std::unique_ptr<Token> collect_id();
        std::unique_ptr<Token> collect_number();  // Common in SQL
};

#endif // SQL_LEXER_H