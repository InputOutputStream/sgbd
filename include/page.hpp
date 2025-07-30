
#ifndef PAGE_HPP

#include "definitions.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <iostream>

class Page 
{
    static const size_t PAGE_SIZE = 4096;

    struct Header {
        uint32_t page_id;
        uint16_t free_space;
        uint16_t slot_count;
        uint32_t checksum;

    };

    Header header;
    std::vector <uint16_t > slot_directory;
    std::vector <uint8_t > data;

    public:
        bool insert_record(const Record& record);
        Record get_record(uint16_t slot_id);
        bool delete_record(uint16_t slot_id);
        void compact_page ();
        bool has_space_for(size_t record_size);
};

#endif // !PAGE_HPP
