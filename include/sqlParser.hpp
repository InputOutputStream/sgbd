#ifndef SQL_PARSER_H
#define SQL_PARSER_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>

#include "sqlLexer.hpp"    
#include "clause.hpp"
#include "ast.hpp"
#include "statement.hpp"


// AST Node types
enum class ExpressionType {
    LITERAL,
    COLUMN_REFERENCE,
    BINARY_OP,
    PARENTHESIZED
};

struct Expression {
    ExpressionType type;
    std::string value;
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    
    Expression(ExpressionType t, std::string v = "") 
        : type(t), value(std::move(v)), left(nullptr), right(nullptr) {}
};


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
        std::unique_ptr<Statement> parse_statements();
        std::unique_ptr<Clause> parse_clause();  
        std::unique_ptr<Expression> parse_expression() ;
        std::unique_ptr<Expression> parse_value_expression() ;
        std::unique_ptr<Expression> parse_binary_expression();
};




#endif // !SQL_PARSER_H
