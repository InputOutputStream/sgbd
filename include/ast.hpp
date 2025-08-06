#ifndef AST
#define AST

    #include <vector>
    #include <iostream>
    #include <string>
    #include <memory>

    #include "clause.hpp"
    #include "sqlLexer.hpp"

class ASTNode {
    public:
        virtual ~ASTNode () = default;
        virtual void accept(ASTVisitor& visitor) = 0;
        virtual std:: string to_string () const = 0;
};


class ASTVisitor {
    public:
        virtual void visit(ASTNode& node) = 0;
        virtual void visit(Expression& expression) = 0;
        virtual void visit(FromClause& from_clause) = 0;
        virtual void visit(WhereClause& where_clause) = 0;
        virtual void visit(OrderByClause& order_by_clause) = 0;
        virtual void visit(LimitClause& limit_clause) = 0;
};



#endif