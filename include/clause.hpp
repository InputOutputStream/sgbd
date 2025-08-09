#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <vector>
#include <iostream>
#include <string>
#include <memory>


#include "sqlLexer.hpp"
#include "ast.hpp"


enum class ClauseType {
    SELECT, FROM, WHERE, GROUP_BY, HAVING, ORDER_BY
};


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
    std::string to_string(){return value;}
};

class Clause: public ASTNode{
    ClauseType type;
    std::vector<std::unique_ptr<Expression>> expression_list;
    std::vector<std::string> arguments;

    public:
        Clause(ClauseType t):type(t){}

        void accept(ASTVisitor& visitor) override {
            visitor.visit(*this);
        }
        std::string to_string() { 
            switch(type){
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
            }   
            return "UNK";
        }
        ClauseType get_type() { return type; }
};


/**
 * <select_list> ::= 
    * 
  | <select_sublist> [ { , <select_sublist> }... ]
*/
class SelectClause : public Clause {
    
    std::vector<std::string> columns;
    
    public:
        SelectClause() : Clause(ClauseType::SELECT) {}

        void add_column(const std::string& col) {
            columns.push_back(col);
        }
        
        std::string to_string() const override {
            std::string result = "SELECT ";
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i > 0) result += ", ";
                result += columns[i];
            }
            return result;
        }
};



/**
 * <group_by_clause> ::= GROUP BY <grouping_column_reference_list>
 */
class GroupClause : public Clause {

    std::vector<std::string> reference_list;
    
    public:
        GroupClause() : Clause(ClauseType::GROUP_BY) {}

        void add_reference(const std::string& col) {
            reference_list.push_back(col);
        }
        
        std::string to_string() const override {
            std::string result = "GROUP BY ";
            for (size_t i = 0; i < reference_list.size(); ++i) {  
                if (i > 0) result += ", ";
                result += reference_list[i];  
            }
            return result;
        }
};



/**
 * <from_clause> ::= FROM <table_reference_list>
        <table_reference> ::= 
            <table_name> [ <correlation_specification> ]
        | <joined_table>
        | ( <table_reference> )
 */
class FromClause : public Clause {
    std::vector<std::string> table_names;
    std::vector<std::string> aliases;

    public:
        FromClause() : Clause(ClauseType::FROM) {}

        void add_table(const std::string& name, const std::string& alias = "") {
            table_names.push_back(name);
            aliases.push_back(alias);
        }

        std::string to_string() const override {
            std::string result = "FROM ";
            for (size_t i = 0; i < table_names.size(); ++i) {
                if (i > 0) result += ", ";
                result += table_names[i];
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
    std::shared_ptr<Expression> condition;

    public:
        WhereClause() : Clause(ClauseType::WHERE) {}

        void set_condition(std::shared_ptr<Expression> cond) {
            condition = std::move(cond);
        }
        
        std::string to_string() const override {
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

    std::vector<std::string> columns;
    std::vector<std::string> directions; // ASC/DESC
    
    public:
        OrderByClause() : Clause(ClauseType::ORDER_BY) {}

        void add_sort_column(const std::string& col, const std::string& dir = "ASC") {
            columns.push_back(col);
            directions.push_back(dir);
        }
        
        std::string to_string() const override {
            std::string result = "ORDER BY ";
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i > 0) result += ", ";
                result += columns[i] + " " + directions[i];
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

};



/***
 * 
 * <joined_table> ::= 
    <table_reference> <join_type> <table_reference> ON <join_condition>
  | <table_reference> NATURAL <join_type> <table_reference>
 */
class JoinClause : public Clause{

};




/**
 * <having_clause> ::= HAVING <search_condition>
 */
struct HavingClause : public Clause {

};




/**
   * <query_expression> ::= 
    <query_term>
  | <query_expression> UNION [ ALL ] <query_term>
  | <query_expression> EXCEPT [ ALL ] <query_term>
   */

#endif // !SQL_ELEMENTS_HPP