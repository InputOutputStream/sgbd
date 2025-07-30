
#ifndef BUFFER_POOL_HPP
#define BUFFER_POOL_HPP 

#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "page.hpp"

class BufferPool 
{
    struct BufferFrame {
        Page* page;
        uint32_t page_id;
        bool is_dirty;
        uint32_t pin_count;
        std:: chrono :: steady_clock :: time_point last_access;
        std::vector <std:: chrono :: steady_clock :: time_point > access_history;
    };
    
    std:: unordered_map <uint32_t , BufferFrame*> page_table;
    std:: vector <BufferFrame > buffer_frames;
    std:: mutex buffer_mutex;
    std:: condition_variable frame_available;

    public:
        Page* get_page(uint32_t page_id);
        void unpin_page(uint32_t page_id , bool is_dirty);
        void flush_all_pages ();
        void prefetch_pages(const std::vector <uint32_t >& page_ids);

    private:
        BufferFrame* evict_page (); // LRU -K algorithm
        void background_flusher ();
        bool try_evict_clean_page ();
};

#endif // !BUFFER_POOL_HPP
