#ifndef SQL_PARSER_H
#define SQL_PARSER_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <functional> 

#include "sqlLexer.hpp"    
#include "clause.hpp"
#include "statement.hpp"


class Parser {
    Lexer& lexer;
    std::unique_ptr<Token> current_token;
    std::unique_ptr<Token> last_token;
    StatementType current_statement_type = StatementType::UNKNOWN;


    enum class ParsingContext {
        STATEMENT_LEVEL,    // Parsing at statement level (looking for clauses)
        CLAUSE_LEVEL        // Parsing within a clause (looking for parameters)
    };
    
    ParsingContext current_context = ParsingContext::STATEMENT_LEVEL;

    private:
        void advance();
        bool match(TokenType type);
        TokenType expected();
        bool is_sql_keyword();
        bool should_stop_parsing();
        bool should_stop_parsing_clause();
        bool is_next_clause_keyword();
        bool should_stop_parsing_parameters();
        bool match_keyword(const std::string& keyword);
        bool is_statement_keyword();
        bool is_clause_keyword_for_statement(StatementType stmt_type);
        void expect_token(TokenType expected, const std::string& error_msg);
        void expect_keyword(const std::string& keyword, const std::string& error_msg);
        
        std::unique_ptr<Clause> parse_main_clause_for_statement(StatementType stmt_type);
        
        template<typename ClauseType>
        void parse_comma_separated_list(ClauseType* clause, 
                                    std::function<void(ClauseType*, const std::string&)> add_item,
                                    const std::string& item_name);
        void set_parsing_context(ParsingContext context){ current_context = context; }


        
    
    public:
        // Stub method declarations
        Parser(Lexer& lex) : lexer(lex) {
            advance();
        }

        std::unique_ptr<Statement> parse_statement();

        std::unique_ptr<Clause> parse_clause();
        std::unique_ptr<Clause> parse_create_clause();
        std::unique_ptr<Clause> parse_create_list();
        std::unique_ptr<Clause> parse_select_clause();  
        std::unique_ptr<Clause> parse_from_clause();  
        std::unique_ptr<Clause> parse_where_clause();  
        std::unique_ptr<Clause> parse_group_by_clause();  
        std::unique_ptr<Clause> parse_order_by_clause();  
        std::unique_ptr<Clause> parse_table_definition_clause();

        std::unique_ptr<Clause> parse_database_definition_clause();
        std::unique_ptr<Clause> parse_having_clause();
        std::unique_ptr<Clause> parse_limit_clause();
        std::unique_ptr<Clause> parse_into_clause();
        std::unique_ptr<Clause> parse_values_clause();
        std::unique_ptr<Clause> parse_set_clause();
        std::unique_ptr<Clause> parse_returning_clause();

        std::unique_ptr<Expression> parse_expression() ;
        std::unique_ptr<Expression> parse_value_expression() ;
        std::unique_ptr<Expression> parse_binary_expression();

        std::unique_ptr<Expression> parse_or_expression();      // Lowest precedence
        std::unique_ptr<Expression> parse_and_expression();     // Middle precedence  
        std::unique_ptr<Expression> parse_not_expression();     // Highest precedence
        std::unique_ptr<Expression> parse_comparison_expression(); // =, <, >, etc.
        std::unique_ptr<Expression> parse_primary_expression(); // Literals, columns, parentheses

};




#endif // !SQL_PARSER_H
