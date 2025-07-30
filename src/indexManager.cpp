
#include "definitions.hpp"
#include "indexManager.hpp"

#include <unordered_map>
#include <vector>
#include <random>
#include <mutex>
#include <condition_variable>
#include <chrono>


// SkipListNode

template <typename Key , typename Value >
SkipListNode(Key k, Value v, int level) : key(k), value(v), forward(level + 1) {

}

Key SkipListNode::get_key () const { 
    return key; 
}

Value SkipListNode::get_value () const { 
    return value; 
}

void SkipListNode::set_forward(int level , SkipListNode* node) { 
    forward[level] = node; 
}

SkipListNode* SkipListNode::get_forward(int level) {
    return forward[level]; 
}



// SkipList

SkipList () : current_level (0), gen(rd()) {
            header = new SkipListNode <Key , Value >(Key{}, Value{}, MAX_LEVEL);
}


template <typename Key , typename Value >
bool insert(Key key , Value value){

}

bool search(Key key , Value& value){

}

bool remove(Key key){

}

void range_query(Key start , Key end , std::vector <Value >& results){

}

int random_level (){

}

std::vector <SkipListNode <Key , Value >*> find_update_vector(Key key){

}
