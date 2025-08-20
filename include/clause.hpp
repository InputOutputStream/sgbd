#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <vector>
#include <iostream>
#include <string>
#include <memory>


#include "sqlLexer.hpp"
#include "ast.hpp"


enum class ClauseType {
   CREATE, SELECT, FROM, WHERE, GROUP_BY, HAVING, ORDER_BY, LIMIT, JOIN, EXPR
};


// AST Node types
enum class ExpressionType {
    LITERAL,
    COLUMN_REFERENCE,
    BINARY_OP,      // For AND, OR, =, <, >, etc.
    UNARY_OP,       // For NOT
    PARENTHESIZED   // For grouped expressions
};

class Clause: public ASTNode{
    ClauseType type;

    public:
        Clause(ClauseType t):type(t){}

        void accept(ASTVisitor& visitor) {
            visitor.visit(*this);
        }
        
        std::string to_string() override { 
            switch(type){
                case ClauseType::CREATE:
                    return "CREATE";
                    break;
                case ClauseType::SELECT:
                    return "SELECT";
                    break;
                case ClauseType::FROM:
                    return "FROM";
                    break;
                case ClauseType::WHERE:
                    return "WHERE";
                    break;
                case ClauseType::GROUP_BY:
                    return "GROUP_BY";
                    break;
                case ClauseType::HAVING:
                    return "HAVING";
                    break;
                case ClauseType::ORDER_BY:
                    return "ORDER_BY";
                    break;
                case ClauseType::LIMIT:
                    return "LIMIT";
                    break;
                case ClauseType::JOIN:
                    return "JOIN";
                    break;
                case ClauseType::EXPR:
                    return "EXPRESSION";
                    break;
            }   
            return "UNK";
        }
        ClauseType get_type() { return type; }
};



struct Expression : Clause {
    ExpressionType type;
    std::string value;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
    Expression(ExpressionType t, std::string v = "") 
        :Clause(ClauseType::EXPR), type(t), value(std::move(v)), left(nullptr), right(nullptr) {
        }
    std::string to_string(){return value;}
};


/**
 * <select_list> ::= 
    * 
  | <select_sublist> [ { , <select_sublist> }... ]
  */

class CreateClause : public Clause {
    
    std::vector<std::string> items;  // Uniformized: was 'columns'
    
    public:
        CreateClause() : Clause(ClauseType::CREATE) {}

        void add_item(const std::string& item) {  // Uniformized: was 'add_column'
            items.push_back(item);
        }
        
        std::string to_string() override {
            std::string result = "CREATE ";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) result += ", ";
                result += items[i];
            }
            return result;
        }
};

class SelectClause : public Clause {
    
    std::vector<std::string> items;  // Uniformized: was 'columns'
    
    public:
        SelectClause() : Clause(ClauseType::SELECT) {}

        void add_item(const std::string& item) {  // Uniformized: was 'add_column'
            items.push_back(item);
        }
        
        std::string to_string() override {
            std::string result = "SELECT ";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) result += ", ";
                result += items[i];
            }
            return result;
        }
};



/**
 * <group_by_clause> ::= GROUP BY <grouping_column_reference_list>
 */
class GroupClause : public Clause {

    std::vector<std::string> items;  // Uniformized: was 'reference_list'
    
    public:
        GroupClause() : Clause(ClauseType::GROUP_BY) {}

        void add_item(const std::string& item) {  // Uniformized: was 'add_reference'
            items.push_back(item);
        }
        
        std::string to_string() override {
            std::string result = "GROUP BY ";
            for (size_t i = 0; i < items.size(); ++i) {  
                if (i > 0) result += ", ";
                result += items[i];  
            }
            return result;
        }
};

class CreateList // Todo impl
{

};

/**
 * <from_clause> ::= FROM <table_reference_list>
        <table_reference> ::= 
            <table_name> [ <correlation_specification> ]
        | <joined_table>
        | ( <table_reference> )
 */
class FromClause : public Clause {
    std::vector<std::string> items;    // Uniformized: was 'table_names'
    std::vector<std::string> aliases;  // Kept as is since it's auxiliary data

    public:
        FromClause() : Clause(ClauseType::FROM) {}

        void add_item(const std::string& item, const std::string& alias = "") {  // Uniformized: was 'add_table'
            items.push_back(item);
            aliases.push_back(alias);
        }

        std::string to_string() override {
            std::string result = "FROM ";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) result += ", ";
                result += items[i];
                if (!aliases[i].empty()) {
                    result += " AS " + aliases[i];
                }
            }
            return result;
        }
};


/**
 * <where_clause> ::= WHERE <search_condition>

<search_condition> ::= 
    <boolean_value_expression>
  | <predicate>
  | <search_condition> AND <search_condition>
  | <search_condition> OR <search_condition>
  | NOT <search_condition>
 */
class WhereClause : public Clause {
    std::unique_ptr<Expression> condition;  // Special case: single expression instead of list

    public:
        WhereClause() : Clause(ClauseType::WHERE) {}

        void set_condition(std::unique_ptr<Expression> cond) {
            condition = std::move(cond);
        }
        
        std::string to_string() override {
            return "WHERE " + (condition ? condition->to_string() : "");
        }
        
};


/**
 * <order_by_clause> ::= ORDER BY <sort_specification_list>

<sort_specification> ::= 
    <sort_key> [ <ordering_specification> ]

<ordering_specification> ::= ASC | DESC
 */
class OrderByClause : public Clause {

    std::vector<std::string> items;      // Uniformized: was 'columns'
    std::vector<std::string> directions; // Kept as is since it's auxiliary data (ASC/DESC)
    
    public:
        OrderByClause() : Clause(ClauseType::ORDER_BY) {}

        void add_item(const std::string& item, const std::string& dir = "ASC") {  // Uniformized: was 'add_sort_column'
            items.push_back(item);
            directions.push_back(dir);
        }
        
        std::string to_string() override {
            std::string result = "ORDER BY ";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) result += ", ";
                result += items[i] + " " + directions[i];
            }
            return result;
        }
};




/**
 * 
 * -- Not in original SQL standard, added later
<limit_clause> ::= LIMIT <unsigned_integer>
-- Or vendor-specific variants like TOP, ROWNUM
 * * */
class LimitClause : public Clause{
    std::vector<std::string> items;  // Uniformized: for consistency

    public:
        LimitClause() : Clause(ClauseType::LIMIT) {}

        void add_item(const std::string& item) {
            items.push_back(item);
        }
        
        std::string to_string() override {
            std::string result = "LIMIT ";
            if (!items.empty()) {
                result += items[0];  // LIMIT typically takes one value
            }
            return result;
        }
};



/***
 * 
 * <joined_table> ::= 
    <table_reference> <join_type> <table_reference> ON <join_condition>
  | <table_reference> NATURAL <join_type> <table_reference>
 */
class JoinClause : public Clause{
    std::vector<std::string> items;  // Uniformized: for consistency

    public:
        JoinClause() : Clause(ClauseType::JOIN) {}

        void add_item(const std::string& item) {
            items.push_back(item);
        }
        
        std::string to_string() override {
            std::string result = "JOIN ";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) result += " ";
                result += items[i];
            }
            return result;
        }
};




/**
 * <having_clause> ::= HAVING <search_condition>
 */
class HavingClause : public Clause {
    std::unique_ptr<Expression> condition;  // Special case: like WHERE, single expression

    public:
        HavingClause() : Clause(ClauseType::HAVING) {}

        void set_condition(std::unique_ptr<Expression> cond) {
            condition = std::move(cond);
        }
        
        std::string to_string() override {
            return "HAVING " + (condition ? condition->to_string() : "");
        }
};




/**
   * <query_expression> ::= 
    <query_term>
  | <query_expression> UNION [ ALL ] <query_term>
  | <query_expression> EXCEPT [ ALL ] <query_term>
   */

#endif // !SQL_ELEMENTS_HPP