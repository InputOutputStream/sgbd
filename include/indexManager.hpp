
#ifndef INDEX_MANAGER_HPP
#define INDEX_MANAGER_HPP

#include <unordered_map>
#include <vector>
#include <random>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "page.hpp"

template <typename Key , typename Value >
class SkipListNode {
    Key key;
    Value value;
    std::vector <SkipListNode*> forward;

    public:
        SkipListNode(Key k, Value v, int level);
        Key get_key ();
        Value get_value ();
        void set_forward(int level , SkipListNode* node);
        SkipListNode* get_forward(int level);
 };

template <typename Key , typename Value >
class SkipList {
    static const int MAX_LEVEL = 16;
    SkipListNode <Key , Value >* header;
    int current_level;
    std:: random_device rd;
    std:: mt19937 gen;

    public:
        SkipList () : current_level (0), gen(rd());
        bool insert(Key key , Value value);
        bool search(Key key , Value& value);
        bool remove(Key key);
        void range_query(Key start , Key end , std::vector <Value >& results);

    private:
        int random_level ();
        std::vector <SkipListNode <Key , Value >*> find_update_vector(Key key);
};


#endif // INDEX_MANAGER_HPP