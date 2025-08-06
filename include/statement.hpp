#ifndef SQL_STATEMENT_HPP
#define SQL_STATEMENT_HPP

#include <vector>
#include <iostream>
#include <string>
#include <memory>

#include "sqlLexer.hpp"
#include "ast.hpp"
#include "clause.hpp"


enum class StatementType{
    SELECT, INSERT, UPDATE, DELETE, CREATE
};

class Statement : public ASTNode {
    StatementType type;
    std::vector<std::unique_ptr<Clause>> clauses;
    
    public:  
        void set_type(StatementType type): type(type){}
        void add_clause(std::unique_ptr<Clause> clause);
        const std::vector<std::unique_ptr<Clause>>& get_clauses() const;
};


 /**
  * <subquery> ::= ( <query_expression> )
  */
struct Subquery : Statement {

};


#endif