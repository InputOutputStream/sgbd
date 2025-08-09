#ifndef SQL_STATEMENT_HPP
#define SQL_STATEMENT_HPP

#include <vector>
#include <iostream>
#include <string>
#include <memory>

#include "sqlLexer.hpp"
#include "clause.hpp"
#include "ast.hpp"


enum class StatementType{
    SELECT, INSERT, UPDATE, DELETE, CREATE
};

class Statement : public ASTNode {
    StatementType type;
    std::vector<std::unique_ptr<Clause>> clauses;
    
    public:  
        void accept(ASTVisitor& visitor) override {
            visitor.visit(*this);
        }
        
        std::string to_string() const override {
            std::string result;
            for (const auto& clause : clauses) {
                result += clause->to_string() + " ";
            }
            return result;
        }
        void set_type(StatementType type){this->type = type;}
        void add_clause(std::unique_ptr<Clause> clause) {
            clauses.push_back(std::move(clause));
        }
        const std::vector<std::unique_ptr<Clause>>& get_clauses() const{
            return clauses;
        };
};


 /**
  * <subquery> ::= ( <query_expression> )
  */
struct Subquery : Statement {

};


#endif