#ifndef CacheLRU_hpp
#define CacheLRU_hpp

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <utility>

#include "CacheBase.hpp"

class CacheLRU : public CacheBase
{
public:
    CacheLRU(uint32_t capacity);
    
    void display_trackers(double time);
    
    void insert(const std::string& key, const std::string& value);
    //bool find(const std::string& key, std::string& value);
    
private:
    const uint32_t CAPACITY;
        
    std::list<std::pair<std::string, std::string>> m_queue;
    std::unordered_map<std::string, decltype(m_queue.begin())> m_map;
    
private: // Trackers
    uint64_t t_hit;
    uint64_t t_search;
    
private:
    void     clean();
    uint64_t get_content_size();
};

#endif /* CacheLRU_hpp */
