#ifndef SQL_PARSER_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>

class ASTNode {
    public:
        virtual ~ASTNode () = default;
        virtual void accept(ASTVisitor& visitor) = 0;
        virtual std:: string to_string () const = 0;
};


class SelectStatement : public ASTNode {

    std::vector <std:: unique_ptr <Expression >> select_list;
    std::unique_ptr <FromClause > from_clause;

    std::unique_ptr <WhereClause > where_clause;

    std::unique_ptr <OrderByClause > order_by_clause;

    std::unique_ptr <LimitClause > limit_clause;

 

    public:
        SelectStatement () = default;

        void set_select_list(std::vector <std:: unique_ptr <Expression >> list)
        {
            select_list = std::move(list);
        }
        
        void set_from_clause(std:: unique_ptr <FromClause > clause) {
             from_clause = std::move(clause);
        }

        void accept(ASTVisitor& visitor) override {
            visitor.visit(*this);
        }  
        std:: string to_string () const override;
};



class SQLParser {
    std::vector <Token > tokens;
    size_t current_token;

    public:
        std:: unique_ptr <ASTNode > parse(const std:: string& sql);
        std:: unique_ptr <SelectStatement > parse_select_statement ();
        std:: unique_ptr <Expression > parse_expression ();
        std:: unique_ptr <WhereClause > parse_where_clause ();

    
    private:
        Token peek() const;
        Token consume ();
        bool match(TokenType type);
        void expect(TokenType type);
};


#endif // !SQL_PARSER_H
