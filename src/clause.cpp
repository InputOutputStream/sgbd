#include <vector>
#include <iostream>
#include <string>
#include <memory>


#include "../include/sqlLexer.hpp"
#include "../include/expression.hpp"
#include "../include/clause.hpp"
#include "../include/statement.hpp"




/**
 * Clause class Implemetation
 */

Clause::Clause(ClauseType t):type(t){}

ClauseType Clause::get_type() const { return type; }

/**
 * Statement class Implemetation
 */

void Statement::add_clause(std::unique_ptr<Clause> clause){

}

const std::vector<std::unique_ptr<Clause>>& Statement::get_clauses() const{
    
}