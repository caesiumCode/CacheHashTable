#ifndef CacheLRU_hpp
#define CacheLRU_hpp

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <utility>

#include "CacheBase.hpp"
#include "CacheBase.cpp"

using queue_it = std::list<std::pair<std::string, std::string>>::iterator;

template<typename HashT = std::hash<std::string>, typename MapT = std::unordered_map<std::string, queue_it, HashT>>
class CacheLRU : public CacheBase<HashT>
{
public:
    CacheLRU(uint32_t capacity);
    
    void display();
    void display_trackers(double time);
    
    void insert(const std::string& key, const std::string& value);
    //bool find(const std::string& key, std::string& value);
    
    uint64_t size() {return m_queue.size();}
    uint64_t content_size();
    uint64_t bookkeeping_overhead() {return bookkeeping_overhead(MapT());}
    
    static std::string get_map_name() {return get_map_name(MapT());}
    static std::string get_map_name(const std::unordered_map<std::string, queue_it, HashT>& map)    {return "std";}
    static std::string get_map_name(const emhash7::HashMap<std::string, queue_it, HashT>& map)      {return "emh";}
    
private:
    const uint32_t CAPACITY;
        
    std::list<std::pair<std::string, std::string>> m_queue;
    MapT m_map;
    
private: // Trackers
    uint64_t t_hit;
    uint64_t t_search;
    
private:
    void     clean();
    
    uint64_t bookkeeping_overhead(const std::unordered_map<std::string, queue_it, HashT>& map);
    uint64_t bookkeeping_overhead(const emhash7::HashMap<std::string, queue_it, HashT>& map);
};

#endif /* CacheLRU_hpp */
