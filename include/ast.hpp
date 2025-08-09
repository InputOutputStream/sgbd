#ifndef AST_HPP
#define AST_HPP

#include <vector>
#include <iostream>
#include <string>
#include <memory>

// Forward declarations
class Expression;
class FromClause;
class WhereClause;
class OrderByClause;
class LimitClause;
class SelectClause;
class GroupClause;
class Clause;
class Statement;
class ASTVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual std::string to_string() const = 0;
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(Statement& node) = 0;
    virtual void visit(Clause& node) = 0;
    virtual void visit(Expression& expression) = 0;
    virtual void visit(FromClause& from_clause) = 0;
    virtual void visit(WhereClause& where_clause) = 0;
    virtual void visit(OrderByClause& order_by_clause) = 0;
    virtual void visit(LimitClause& limit_clause) = 0;
    virtual void visit(SelectClause& select_clause) = 0;
    virtual void visit(GroupClause& group_clause) = 0;
};

#endif