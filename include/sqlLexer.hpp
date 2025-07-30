#ifndef SQL_LEXER_H
#define SQL_LEXER_H


#include "definitions.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <string>
#include <memory>

typedef struct TOKEN_STRUCT
{
    enum{
         TOKEN_ID,
         TOKEN_EQUALS,
         TOKEN_STRING,
         TOKEN_SEMI,
         TOKEN_COMMA,
         TOKEN_LPAREN,
         TOKEN_RPAREN,
         TOKEN_LBRACE,
         TOKEN_RBRACE,
         TOKEN_LSBRACE,
         TOKEN_RSBRACE,
         TOKEN_EOF,
    }type;
    
    char *value;

}Token;


class Lexer{

    std::string content;
    char *caratcers;
    int lenght;

    public:
        Lexer(std::string &content);
        void lexer_advance();
        void lexer_skip_caracter(char c);
        std:: unique_ptr<Token> lexer_get_next_token();
        std:: unique_ptr<Token> lexer_collect_string();
        std:: unique_ptr<Token> lexer_collect_id();


    private:
        std:: unique_ptr<Token> _lexer_advance_with_token(Token &token);

};

#endif // !SQL_LEXER_H