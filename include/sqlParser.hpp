#ifndef SQL_PARSER_H
#define SQL_PARSER_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>

#include "sqlLexer.hpp"    
#include "clause.hpp"
#include "statement.hpp"


class Parser {
    Lexer& lexer;
    std::unique_ptr<Token> current_token;
    std::unique_ptr<Token> last_token;

    private:
        void advance();
        bool match(TokenType type);
        void expect(TokenType type);
        TokenType expected();
    
    public:
        Parser(Lexer& lex) : lexer(lex) {
            advance();
        }
        std::unique_ptr<Statement> parse_statement();

        std::unique_ptr<Clause> parse_clause();
        std::unique_ptr<Clause> parse_select_clause();  
        std::unique_ptr<Clause> parse_from_clause();  
        std::unique_ptr<Clause> parse_where_clause();  
        std::unique_ptr<Clause> parse_group_by_clause();  
        std::unique_ptr<Clause> parse_order_by_clause();  

        std::unique_ptr<Expression> parse_expression() ;
        std::unique_ptr<Expression> parse_value_expression() ;
        std::unique_ptr<Expression> parse_binary_expression();
};




#endif // !SQL_PARSER_H
