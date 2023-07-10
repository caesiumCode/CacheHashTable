#ifndef SOA_CacheLRU_hpp
#define SOA_CacheLRU_hpp

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <utility>

#include "CacheBase.hpp"
#include "emhash7.hpp"

class SOA_CacheLRU : public CacheBase
{
public:
    SOA_CacheLRU(uint32_t capacity);
    
    void display();
    void display_trackers(double time);
    
    void insert(const std::string& key, const std::string& value);
    //bool find(const std::string& key, std::string& value);
    
private:
    const uint32_t CAPACITY;
        
    std::list<std::pair<std::string, std::string>> m_queue;
    emhash7::HashMap<std::string, decltype(m_queue.begin())> m_map;
    
private: // Trackers
    uint64_t t_hit;
    uint64_t t_search;
    
private:
    void     clean();
    uint64_t get_content_size();
};

#endif /* SOA_CacheLRU_hpp */
