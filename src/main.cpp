#include <iostream>
#include "sqlLexer.hpp"
#include <memory>

int main(){

    std::string text = "Select age from Student where name = \"Frank\" and age = 34 ";
    Lexer lex(text);
    std::shared_ptr<Token> type;

    do{
        type = lex.get_next_token();
        std::cout<<"Value: "<<type->value<<std::endl;
    }while(type->type != TokenType::END_FILE);
    return 0;
}

//lexer ok