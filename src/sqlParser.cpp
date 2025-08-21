#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <set>
#include <functional> 

#include "sqlLexer.hpp"
#include "statement.hpp"
#include "clause.hpp"
#include "sqlParser.hpp"

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::string to_uppercase(const std::string& word) {
    std::string result = word;
    for(char &c: result) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return result;
}

// ============================================================================
// KEYWORD DEFINITIONS (centralized)
// ============================================================================

namespace Keywords {
    const std::set<std::string> ALL_KEYWORDS = {
        // DDL Keywords
        "CREATE", "DROP", "ALTER", "TABLE", "DATABASE", "INDEX",
        // DML Keywords  
        "SELECT", "INSERT", "UPDATE", "DELETE", 
        // Clause Keywords
        "FROM", "WHERE", "ORDER", "GROUP", "HAVING", "LIMIT",
        "INTO", "VALUES", "SET", "RETURNING",
        // Expression Keywords - ADD THESE:
        "AND", "OR", "NOT", "LIKE", "IN", "BETWEEN", "IS", "NULL",
        "DISTINCT", "AS",
        // Other Keywords
        "BY", "ASC", "DESC"};
    
    const std::set<std::string> STATEMENT_KEYWORDS = {
        "CREATE", "SELECT", "INSERT", "UPDATE", "DELETE", "DROP", "ALTER"
    };
    
    const std::unordered_map<StatementType, std::set<std::string>> CLAUSE_KEYWORDS = {
        {StatementType::CREATE, {"TABLE", "DATABASE", "INDEX"}},
        {StatementType::SELECT, {"FROM", "WHERE", "GROUP", "HAVING", "ORDER", "LIMIT"}},
        {StatementType::INSERT, {"INTO", "VALUES", "RETURNING"}},
        {StatementType::UPDATE, {"SET", "WHERE", "RETURNING"}},
        {StatementType::DELETE, {"FROM", "WHERE", "RETURNING"}}
    };
    
    const std::unordered_map<std::string, StatementType> STATEMENT_TYPES = {
        {"CREATE", StatementType::CREATE},
        {"SELECT", StatementType::SELECT},
        {"INSERT", StatementType::INSERT},
        {"UPDATE", StatementType::UPDATE},
        {"DELETE", StatementType::DELETE}
    };
}

// ============================================================================
// PARSER HELPER METHODS  
// ============================================================================

void Parser::advance() {
    last_token = std::move(current_token);
    current_token = lexer.get_next_token();
}

bool Parser::match(TokenType expected) {
    return current_token && current_token->type == expected;
}

bool Parser::match_keyword(const std::string& keyword) {
    return match(TokenType::ID) && to_uppercase(current_token->value) == keyword;
}

bool Parser::is_sql_keyword() {
    if (!match(TokenType::ID)) return false;
    return Keywords::ALL_KEYWORDS.find(to_uppercase(current_token->value)) != Keywords::ALL_KEYWORDS.end();
}

bool Parser::is_statement_keyword() {
    if (!match(TokenType::ID)) return false;
    return Keywords::STATEMENT_KEYWORDS.find(to_uppercase(current_token->value)) != Keywords::STATEMENT_KEYWORDS.end();
}

bool Parser::is_clause_keyword_for_statement(StatementType stmt_type) {
    if (!match(TokenType::ID)) return false;
    
    auto it = Keywords::CLAUSE_KEYWORDS.find(stmt_type);
    if (it == Keywords::CLAUSE_KEYWORDS.end()) return false;

    return it->second.find(to_uppercase(current_token->value)) != it->second.end();
}


bool Parser::should_stop_parsing() {
   // Only stop on statement terminators
    return match(TokenType::SEMI) || match(TokenType::END_FILE);
}

bool Parser::should_stop_parsing_parameters() { 
    // When parsing parameters, stop on statement terminators OR clause keywords
    return match(TokenType::SEMI) || 
           match(TokenType::END_FILE) || 
           is_clause_keyword_for_statement(current_statement_type);
}


void Parser::expect_token(TokenType expected, const std::string& error_msg) {
    if (!match(expected)) {
        throw std::runtime_error(error_msg + " (found: " + 
            (current_token ? current_token->value : "EOF") + ")");
    }
}

void Parser::expect_keyword(const std::string& keyword, const std::string& error_msg) {
    if (!match_keyword(keyword)) {
        throw std::runtime_error(error_msg + " (found: " + 
            (current_token ? current_token->value : "EOF") + ")");
    }
}



// ============================================================================
// PARSING UTILITIES (DRY principle)
// ============================================================================

template<typename ClauseType>
void Parser::parse_comma_separated_list(ClauseType* clause, 
                                       std::function<void(ClauseType*, const std::string&)> add_item,
                                       const std::string& item_name) {
    // Set context at the beginning
    set_parsing_context(ParsingContext::CLAUSE_LEVEL);
    
    do {
        if(!(match(TokenType::ID) || match(TokenType::NUMBER)))
        {
            throw std::runtime_error(" Expected " + item_name);
        }
        
        std::string item = current_token->value;
        advance();
        add_item(clause, item);
        
        if (match(TokenType::COMMA)) {
            advance();
            continue;
        } else {
            break;
        }
    } while (!should_stop_parsing_parameters());
    
    // Reset context when done
    set_parsing_context(ParsingContext::STATEMENT_LEVEL);
}

// ============================================================================
// MAIN PARSING METHODS
// ============================================================================

std::unique_ptr<Statement> Parser::parse_statement() {
    auto statement = std::make_unique<Statement>();

    expect_token(TokenType::ID, "Expected SQL statement keyword");

    std::string keyword = to_uppercase(current_token->value);
    auto it = Keywords::STATEMENT_TYPES.find(keyword);
    
    if (it == Keywords::STATEMENT_TYPES.end()) {
        throw std::runtime_error("Unknown SQL keyword: " + keyword);
    }

    current_statement_type = it->second;
    statement->set_type(it->second);

    // Parse the main clause for this statement type
    auto main_clause = parse_main_clause_for_statement(it->second);
    if (main_clause) {
        statement->add_clause(std::move(main_clause));
    }

    // Parse additional clauses at statement level
    set_parsing_context(ParsingContext::STATEMENT_LEVEL);
    
    while (!should_stop_parsing()) {      
        auto clause = parse_clause();
        if (clause) {
            statement->add_clause(std::move(clause));
        } else {
            throw std::runtime_error("Unexpected token: " + current_token->value);
            //advance();
        }
    }
    
    return statement;
}

std::unique_ptr<Clause> Parser::parse_main_clause_for_statement(StatementType stmt_type) {
    switch (stmt_type) {
        case StatementType::CREATE:
            return parse_create_clause();
            break;
        case StatementType::SELECT:
            return parse_select_clause();
            break;
        case StatementType::INSERT:
            //return parse_insert_clause();
            break; 

        case StatementType::UPDATE:
            //return parse_update_clause();        
            break; 
        case StatementType::DELETE:
            //return parse_delete_clause();
            break;
        default:
            throw std::runtime_error("Unsupported statement type");
            break;
    }

    return nullptr;  
}

std::unique_ptr<Clause> Parser::parse_clause() {
    if (!match(TokenType::ID)) return nullptr;
    std::string clause_keyword = to_uppercase(current_token->value);
    
    //std::cout << "Parsing clause: [" << clause_keyword << "]" << std::endl;

    // Universal clauses (work with multiple statement types)
    if (clause_keyword == "WHERE") return parse_where_clause();
    if (clause_keyword == "FROM") return parse_from_clause();
    if (clause_keyword == "AND" || clause_keyword == "OR") return parse_binary_expression();
    if (clause_keyword == "NOT") return parse_not_expression();
    

    // Statement-specific clauses
    switch (current_statement_type) {
        case StatementType::CREATE:
            if (clause_keyword == "TABLE")    return parse_table_definition_clause();
            if (clause_keyword == "DATABASE") return parse_table_definition_clause();
            break;

        case StatementType::SELECT:
            if (clause_keyword == "ORDER")   return parse_order_by_clause();
            if (clause_keyword == "GROUP")   return parse_group_by_clause();
            if (clause_keyword == "HAVING")  return parse_having_clause();
            if (clause_keyword == "LIMIT")   return parse_limit_clause();
            break;

        case StatementType::INSERT:
            if (clause_keyword == "INTO")      return parse_into_clause();
            if (clause_keyword == "VALUES")    return parse_values_clause();
            if (clause_keyword == "RETURNING") return parse_returning_clause();
            break;

        case StatementType::UPDATE:
            if (clause_keyword == "SET")       return parse_set_clause();
            if (clause_keyword == "RETURNING") return parse_returning_clause();
            break;

        case StatementType::DELETE:
            if (clause_keyword == "RETURNING") return parse_returning_clause();
            break;

        default:
            break;
    }

    // If we reach here, it's an unrecognized clause keyword
    std::cerr << "Unrecognized clause keyword: " << clause_keyword << std::endl;
    return nullptr;
}

// ============================================================================
// SPECIFIC CLAUSE PARSING METHODS
// ============================================================================

std::unique_ptr<Clause> Parser::parse_create_clause() {
    advance(); // consume CREATE
    set_parsing_context(ParsingContext::CLAUSE_LEVEL);
    auto create_clause = std::make_unique<CreateClause>();

    expect_token(TokenType::ID, "Expected object type after CREATE");
    std::string object_type = to_uppercase(current_token->value);
    advance();

    if (object_type == "TABLE") {
        create_clause->set_is_table(true);
    }
    else if (object_type == "DATABASE") {
        create_clause->set_is_table(false);
    } else {
        throw std::runtime_error("Unsupported CREATE type: " + object_type);
    }

    expect_token(TokenType::ID, "Expected name after CREATE " + object_type);
    create_clause->set_name(current_token->value);
    advance();

    // For DATABASE, we're done (no column definitions)
    if (object_type == "DATABASE") {
        set_parsing_context(ParsingContext::STATEMENT_LEVEL);
        return create_clause;
    }

    // For TABLE, check if there are column definitions
    if (!match(TokenType::LPAREN)) { 
        set_parsing_context(ParsingContext::STATEMENT_LEVEL);
        return create_clause; // Table without column definitions
    }

    // Parse column definitions
    advance(); // consume '('
    
    do {
        std::string column;
        std::vector<std::string> col_attributes;
        
        expect_token(TokenType::ID, "Expected column name");
        if (!is_sql_keyword()) {
            column = current_token->value;
            advance();
        } else {
            throw std::runtime_error("Column name cannot be a SQL keyword: " + current_token->value);
        }

        // Parse column attributes (data type, constraints, etc.)
        while (!match(TokenType::COMMA) && !match(TokenType::RPAREN)) {
            if (match(TokenType::ID)) {
                col_attributes.push_back(current_token->value);
                advance();
            } else if (match(TokenType::LPAREN)) {
                // Handle data type with size: VARCHAR(255)
                advance(); // consume '('
                if (match(TokenType::NUMBER)) {
                    col_attributes.back() += "(" + current_token->value + ")";
                    advance();
                }
                expect_token(TokenType::RPAREN, "Expected ')' after data type size");
                advance();
            } else {
                break;
            }
        }
        
        // Add column and its attributes
        create_clause->add_item(column, col_attributes);
        
        if (match(TokenType::COMMA)) {
            advance();
            continue;
        } else if (match(TokenType::RPAREN)) {
            advance(); // consume ')'
            break;
        } else {
            throw std::runtime_error("Expected ',' or ')' in column definition");
        }
        
    } while (true);
    
    set_parsing_context(ParsingContext::STATEMENT_LEVEL);
    return create_clause;
}

std::unique_ptr<Clause> Parser::parse_select_clause() {
    advance(); // consume SELECT
    set_parsing_context(ParsingContext::CLAUSE_LEVEL);
    
    auto select_clause = std::make_unique<SelectClause>();
    
    // Handle DISTINCT keyword
    if (match_keyword("DISTINCT")) {
        advance();
        select_clause->set_distinct(true);
    }

    if (match(TokenType::STAR)) { 
        select_clause->add_item("*");
        advance();
        set_parsing_context(ParsingContext::STATEMENT_LEVEL); // Reset context
        return select_clause;
    }

    do {
        expect_token(TokenType::ID, "Expected column name in SELECT clause");
        std::string column = current_token->value;
        advance();

        std::string alias = "";
        if (match_keyword("AS")) {
            advance(); // consume AS
            expect_token(TokenType::ID, "Expected alias after AS");
            alias = current_token->value;
            advance();
        }else if (match(TokenType::ID) && !is_sql_keyword()) { // Check if next token is not a keyword
            alias = current_token->value; // handle alias without AS
            advance();
        } 
            
        // Add column and alias to the select clause
        select_clause->add_item(column, alias);
        
        if (match(TokenType::COMMA)) {
            advance();
            continue;
        } else {
            break; // No more columns
        }
    } while (!should_stop_parsing_parameters()); // Use parameter-specific method
    
    set_parsing_context(ParsingContext::STATEMENT_LEVEL); // Reset context
    return select_clause;
}

std::unique_ptr<Clause> Parser::parse_from_clause() {
    advance(); // consume FROM
    set_parsing_context(ParsingContext::CLAUSE_LEVEL);
    
    auto from_clause = std::make_unique<FromClause>();
    
    do {
        expect_token(TokenType::ID, "Expected table name in FROM clause");
        std::string table_name = current_token->value;
        advance();
        
        std::string alias = "";
        if (match_keyword("AS")) {
            advance(); // consume AS
            expect_token(TokenType::ID, "Expected alias after AS");
            alias = current_token->value;
            advance();
        }
        
        from_clause->add_item(table_name, alias);
        
        if (match(TokenType::COMMA)) {
            advance();
            continue;
        } else {
            break;
        }
    } while (!should_stop_parsing_parameters());

    set_parsing_context(ParsingContext::STATEMENT_LEVEL);
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
    expect_keyword("BY", "Expected BY after ORDER");
    advance(); // consume BY
    
    set_parsing_context(ParsingContext::CLAUSE_LEVEL); // Add context management
    
    auto order_clause = std::make_unique<OrderByClause>();
    
    do {
        expect_token(TokenType::ID, "Expected column name in ORDER BY clause");
        std::string column = current_token->value;
        advance();
        
        std::string direction = "ASC";
        if (match_keyword("ASC") || match_keyword("DESC")) {
            direction = to_uppercase(current_token->value);
            advance();
        }
        
        order_clause->add_item(column, direction);
        
        if (match(TokenType::COMMA)) {
            advance();
            continue;
        } else {
            break;
        }
    } while (!should_stop_parsing_parameters()); // Use parameter-specific method
    
    set_parsing_context(ParsingContext::STATEMENT_LEVEL); // Reset context
    return order_clause;
}

std::unique_ptr<Clause> Parser::parse_group_by_clause() {
    advance(); // consume GROUP
    expect_keyword("BY", "Expected BY after GROUP");
    advance(); // consume BY
    
    auto group_clause = std::make_unique<GroupClause>();
    
    parse_comma_separated_list<GroupClause>(
        group_clause.get(),
        [](GroupClause* clause, const std::string& item) { clause->add_item(item); },
        "column name"
    );
    
    return group_clause;
}

// ============================================================================
// STUB IMPLEMENTATIONS (to be completed)
// ============================================================================

std::unique_ptr<Clause> Parser::parse_table_definition_clause() {
    advance(); // consume TABLE
    set_parsing_context(ParsingContext::CLAUSE_LEVEL);
    
    auto create_clause = std::make_unique<CreateClause>();
    
    expect_token(TokenType::ID, "Expected table name in create clause");
    std::string table_name = current_token->value;
    advance();
    create_clause->add_item(table_name, std::vector<std::string>{});
        
    do {
            if(match(TokenType::LPAREN)){
                advance(); // consume LPAREN
            } else {
                break;
            }    

        if (match(TokenType::COMMA)) {
            advance();
            continue;
        } else {
            break;
        }
    } while (!should_stop_parsing_parameters());

    set_parsing_context(ParsingContext::STATEMENT_LEVEL);
    return create_clause;
}

// std::unique_ptr<Clause> Parser::parse_database_definition_clause() {
//     // TODO: Implement database definition parsing  
//     advance(); // consume DATABASE for now
//     return nullptr;
// }

std::unique_ptr<Clause> Parser::parse_having_clause() {
    advance(); // consume HAVING
    auto having_clause = std::make_unique<HavingClause>();
    auto expr = parse_expression(); // Use full expression parser
    having_clause->set_condition(std::move(expr));
    return having_clause;
}

std::unique_ptr<Clause> Parser::parse_limit_clause() {
    advance(); // consume LIMIT
    auto limit_clause = std::make_unique<LimitClause>();
    expect_token(TokenType::NUMBER, "Expected number after LIMIT");
    std::string limit_value = current_token->value;
    advance();
    limit_clause->add_item(limit_value);
    return limit_clause;
}

std::unique_ptr<Clause> Parser::parse_into_clause() {
    // TODO: Implement INTO clause parsing
    advance(); // consume INTO for now
    return nullptr;
}

std::unique_ptr<Clause> Parser::parse_values_clause() {
    // TODO: Implement VALUES clause parsing
    advance(); // consume VALUES for now
    return nullptr;
}

std::unique_ptr<Clause> Parser::parse_set_clause() {
    // TODO: Implement SET clause parsing
    advance(); // consume SET for now
    return nullptr;
}

std::unique_ptr<Clause> Parser::parse_returning_clause() {
    // TODO: Implement RETURNING clause parsing
    advance(); // consume RETURNING for now
    return nullptr;
}

// ============================================================================
// EXPRESSION PARSING (unchanged)
// ============================================================================

std::unique_ptr<Expression> Parser::parse_expression() {
    return parse_or_expression();
}

std::unique_ptr<Expression> Parser::parse_or_expression() {
    auto left = parse_and_expression();
    
    while (match_keyword("OR")) {
        std::string op = current_token->value;
        advance();
        auto right = parse_and_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, op);
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        left = std::move(binary_expr);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parse_and_expression() {
    auto left = parse_not_expression();
    
    while (match_keyword("AND")) {
        std::string op = current_token->value;
        advance();
        auto right = parse_not_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, op);
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        left = std::move(binary_expr);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parse_not_expression() {
    if (match_keyword("NOT")) {
        std::string op = current_token->value;
        advance();
        auto operand = parse_not_expression(); // Right-associative
        
        auto unary_expr = std::make_unique<Expression>(ExpressionType::UNARY_OP, op);
        unary_expr->left = std::move(operand);
        return unary_expr;
    }
    
    return parse_comparison_expression();
}

std::unique_ptr<Expression> Parser::parse_comparison_expression() {
    auto left = parse_primary_expression();
    
    // Handle comparison operators: =, <, >, <=, >=, <>, LIKE, etc.
    if (match(TokenType::EQUALS) || match_keyword("LIKE") || 
        match_keyword("IN") || match_keyword("BETWEEN")) {

        std::string op = current_token->value;
        if (match(TokenType::ID)) {
            op = to_uppercase(op);
        }

        advance();

        // Handle IS NULL / IS NOT NULL
        if (op == "IS") {
            if (match_keyword("NOT")) {
                advance();
                expect_keyword("NULL", "Expected NULL after IS NOT");
                op = "IS NOT NULL";
                advance();
                // No right operand for IS NULL/IS NOT NULL
                auto unary_expr = std::make_unique<Expression>(ExpressionType::UNARY_OP, op);
                unary_expr->left = std::move(left);
                return unary_expr;
            } else {
                expect_keyword("NULL", "Expected NULL after IS");
                op = "IS NULL";
                advance();
                auto unary_expr = std::make_unique<Expression>(ExpressionType::UNARY_OP, op);
                unary_expr->left = std::move(left);
                return unary_expr;
            }
        }
        auto right = parse_primary_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, op);
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        return binary_expr;
    }else if(match(TokenType::GREATER_EQUAL)){
        advance(); // consume GREATER_EQUAL
        auto right = parse_primary_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, ">=");
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        return binary_expr;
    }else if(match(TokenType::LESS_EQUAL)){
         advance(); // consume LESS_EQUAL
        auto right = parse_primary_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, "<=");
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        return binary_expr;
    }else if(match(TokenType::GREATER_THAN)){
         advance(); // consume GREATER_THAN
        auto right = parse_primary_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, ">");
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        return binary_expr;
    }else if(match(TokenType::LESS_THAN)){
        advance(); // consume LESS_THAN
        auto right = parse_primary_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, "<");
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        return binary_expr; 
    }else if(match(TokenType::NOT_EQUAL)){
        advance(); // consume NOT_EQUAL
        auto right = parse_primary_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, "<>");
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        return binary_expr;
    } 
    
    // If no comparison operator, return the left operand as is
    return left;
}

std::unique_ptr<Expression> Parser::parse_primary_expression() {
    // Handle parentheses
    if (match(TokenType::LPAREN)) {
        advance();
        auto expr = parse_expression(); // Recursive call for full expression parsing
        expect_token(TokenType::RPAREN, "Expected ')'");
        advance();
        return expr;
    }
    
    // Handle literals and column references
    if (match(TokenType::NUMBER) || match(TokenType::STRING)) {
        auto literal = std::make_unique<Expression>(ExpressionType::LITERAL, current_token->value);
        advance();
        return literal;
    }
    
    if (match(TokenType::ID)) {
        std::string value = current_token->value;
        advance();
        
        // Check for SQL keywords used as values (like NULL)
        if (to_uppercase(value) == "NULL") {
            return std::make_unique<Expression>(ExpressionType::LITERAL, "NULL");
        }

        return std::make_unique<Expression>(ExpressionType::COLUMN_REFERENCE, current_token->value);
    }
    
    throw std::runtime_error("Expected expression");
}

std::unique_ptr<Expression> Parser::parse_binary_expression() {
    auto left = parse_comparison_expression(); // Handle =, <, >, etc.
    
    // Handle AND/OR with left-associativity
    while (match_keyword("AND") || match_keyword("OR")) {
        std::string op = current_token->value;
        advance();
        auto right = parse_comparison_expression();
        
        auto binary_expr = std::make_unique<Expression>(ExpressionType::BINARY_OP, op);
        binary_expr->left = std::move(left);
        binary_expr->right = std::move(right);
        left = std::move(binary_expr);
    }
    
    return left;
}