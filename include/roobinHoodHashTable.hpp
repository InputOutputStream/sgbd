#ifndef ROBIN_HOOD_HASH_TABLE_H
#define ROBIN_HOOD_HASH_TABLE_H

#include <cstdint>
#include <cstdlib>
#include <vector>

template <typename Key , typename Value >
class RobinHoodHashTable {
    struct Entry {
        Key key;
        Value value;
        uint32_t hash;
        uint32_t distance; // Distance from ideal position
        bool is_occupied;

        Entry () : distance (0), is_occupied(false) {}
    };

    std::vector <Entry > table;
    size_t capacity;
    size_t size;
    static const double MAX_LOAD_FACTOR = 0.75;

    public:
        RobinHoodHashTable(size_t initial_capacity = 1024)
        : capacity(initial_capacity), size (0) {

            table.resize(capacity);
        }

        bool insert(const Key& key , const Value& value);
        bool find(const Key& key , Value& value);
        bool remove(const Key& key);
        double load_factor () const { return (double)size / capacity; }

    private:
        uint32_t hash_function(const Key& key);
        void resize ();
        size_t probe_distance(size_t hash , size_t slot_index);
        void insert_entry(Entry entry);
};



#endif // !ROBIN_HOOD_HASH_TABLE_H
