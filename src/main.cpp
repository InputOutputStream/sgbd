#include <iostream>
#include "sqlLexer.hpp"
#include "sqlParser.hpp"
#include <memory>

int main(){

    std::string text = "Select age, sex from Student, Teachers where name = \"Frank\" and age = 34 group by Name, Class;";
    Lexer lex(text);
    std::shared_ptr<Token> type;

    // do{
    //     type = lex.get_next_token();
    //     std::cout<<"Value: "<<type->value<<std::endl;
    // }while(type->type != TokenType::END_FILE);

    Parser p(lex);

    auto s = p.parse_statement();
    const auto& k = s->get_clauses();  
    
    std::cout << s->to_string() <<"\n";      
    
    // for(const auto& c: k){         
    //     std::cout << c->to_string() <<"\n";      
    // }
        
    return 0;
}

//lexer ok