#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>

#include "sqlLexer.hpp"
#include "statement.hpp"
#include "clause.hpp"
#include "sqlParser.hpp"

std::string toupper(const std::string& word) {
    std::string result = word;
    for(char &c: result) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return result;
}

void Parser::advance() {
    last_token = std::move(current_token);
    current_token = lexer.get_next_token();
}

bool Parser::match(TokenType received) {
    return current_token && current_token->type == received;
}

std::unique_ptr<Statement> Parser::parse_statement() {
    auto statement = std::make_unique<Statement>();
    
    if (!match(TokenType::ID)) {
        throw std::runtime_error("Expected SQL keyword");
    }
    
    // Map keywords to statement types
    static const std::unordered_map<std::string, StatementType> keywords = {
        {"SELECT", StatementType::SELECT},
        {"INSERT", StatementType::INSERT}, 
        {"UPDATE", StatementType::UPDATE},
        {"DELETE", StatementType::DELETE}
    };
    
    std::string keyword = toupper(current_token->value);
    auto it = keywords.find(keyword);
    
    if (it == keywords.end()) {
        throw std::runtime_error("Unknown SQL keyword: " + keyword);
    }
    
    statement->set_type(it->second);

    // Parse first clause based on statement type
    if (keyword == "SELECT") {
        auto select_clause = parse_select_clause();
        statement->add_clause(std::move(select_clause));
    }
    
    // Pour INSERT/UPDATE/DELETE, ne pas parser SELECT clause
    
    // Parse remaining clauses...
    while (!match(TokenType::SEMI) && !match(TokenType::END_FILE)) {
        auto clause = parse_clause();
        if (clause) {
            statement->add_clause(std::move(clause));
        }
    }
    
    return statement;
}

std::unique_ptr<Clause> Parser::parse_clause() {
    if (!match(TokenType::ID)) {
        advance(); // skip unknown tokens
        return nullptr;
    }
    
    std::string clause_keyword = toupper(current_token->value);
    
    if (clause_keyword == "FROM") {
        return parse_from_clause();
    } else if (clause_keyword == "WHERE") {
        return parse_where_clause();
    } else if (clause_keyword == "ORDER") {
        return parse_order_by_clause();
    } else if (clause_keyword == "GROUP") {
        return parse_group_by_clause();
    }
    
    advance(); // skip unknown clause
    return nullptr;
}



std::unique_ptr<Clause> Parser::parse_select_clause(){
    advance(); // consume select
    
    auto select_clause = std::make_unique<SelectClause>();
    
    if (match(TokenType::STAR)) { 
        select_clause->add_column("*");
        advance();
        return select_clause;
    }

    do {
        if (match(TokenType::ID)) {
            std::string column = current_token->value;
            advance();
            select_clause->add_column(column);
        }
        
        if (match(TokenType::COMMA)) {
            advance();
        } else {
            break;
        }
    } while (true);
    
    return select_clause;
}


std::unique_ptr<Clause> Parser::parse_from_clause() {
    advance(); // consume FROM
    auto from_clause = std::make_unique<FromClause>();
    
    do {
        if (!match(TokenType::ID)) {
            throw std::runtime_error("Expected table name");
        }
        
        std::string table_name = current_token->value;
        advance();
        
        std::string alias = "";
        if (match(TokenType::ID) && toupper(current_token->value) == "AS") {
            advance(); // consume AS
            if (match(TokenType::ID)) {
                alias = current_token->value;
                advance();
            }
        }
        
        from_clause->add_table(table_name, alias);
        
        if (match(TokenType::COMMA)) {
            advance();
        } else {
            break;
        }
    } while (true);
    
    return from_clause;
}


std::unique_ptr<Clause> Parser::parse_where_clause() {
    advance(); // consume WHERE
    auto where_clause = std::make_unique<WhereClause>();
    auto expr = parse_binary_expression();
    where_clause->set_condition(std::move(expr));
            
    return where_clause;
}

std::unique_ptr<Clause> Parser::parse_order_by_clause() {
    advance(); // consume ORDER
    if (match(TokenType::ID) && toupper(current_token->value) == "BY") {
        advance(); // consume BY
    }
    
    auto order_clause = std::make_unique<OrderByClause>();
    
    do {
        if (match(TokenType::ID)) {
            std::string column = current_token->value;
            advance();
            
            std::string direction = "ASC";
            if (match(TokenType::ID)) {
                std::string dir = toupper(current_token->value);
                if (dir == "ASC" || dir == "DESC") {
                    direction = dir;
                    advance();
                }
            }
            
            order_clause->add_sort_column(column, direction);
        }
        
        if (match(TokenType::COMMA)) {
            advance();
        } else {
            break;
        }
    } while (true);
    
    return order_clause;
}


std::unique_ptr<Clause> Parser::parse_group_by_clause(){
    advance(); // consume GROUP
    if (match(TokenType::ID) && toupper(current_token->value) == "BY") {
        advance(); // consume BY
    }
    
    auto group_clause = std::make_unique<GroupClause>();
    
    do {
        if (match(TokenType::ID)) {
            std::string column = current_token->value;
            advance();
            group_clause->add_reference(column);
        }
        
        if (match(TokenType::COMMA)) {
            advance();
        } else {
            break;
        }
    } while (true);
    
    return group_clause;
}


std::unique_ptr<Expression> Parser::parse_expression() {
    return parse_binary_expression();
}

std::unique_ptr<Expression> Parser::parse_value_expression() {
    if (match(TokenType::LPAREN)) {
        advance();
        auto expr = parse_value_expression();
        
        if (!match(TokenType::RPAREN)) {
            throw std::runtime_error("Expected ')'");
        }
        advance();
        
        auto paren_expr = std::make_unique<Expression>(ExpressionType::PARENTHESIZED);
        paren_expr->left = std::move(expr);
        return paren_expr;
    }
    
    if (match(TokenType::NUMBER) || match(TokenType::STRING)) {
        auto literal = std::make_unique<Expression>(ExpressionType::LITERAL, current_token->value);
        advance();
        return literal;
    }
    
    if (match(TokenType::ID)) {
        auto column = std::make_unique<Expression>(ExpressionType::COLUMN_REFERENCE, current_token->value);
        advance();
        return column;
    }
    
    throw std::runtime_error("Expected expression");
}

std::unique_ptr<Expression> Parser::parse_binary_expression() {
    auto left = parse_value_expression();
    
    if (match(TokenType::EQUALS)) {
        std::string op = current_token->value;
        advance();
        auto right = parse_value_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, op);
        binary_expr->left = std::move(left); 
        binary_expr->right = std::move(right);
        return binary_expr;
    }
    
    return left;
}