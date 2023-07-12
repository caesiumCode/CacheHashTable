#ifndef CacheLRU_hpp
#define CacheLRU_hpp

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <utility>

#include "CacheBase.hpp"
#include "CacheBase.cpp"

struct ListNode
{
    std::string key;
    std::string value;
    
    ListNode* previous;
    ListNode* next;
};

using queue_t = ListNode*;

template<typename HashT = std::hash<std::string>, typename MapT = std::unordered_map<std::string, queue_t, HashT>>
class CacheLRU : public CacheBase<HashT>
{
public:
    CacheLRU(uint32_t capacity);
    ~CacheLRU();
    
    void display();
    void display_trackers(double time);
    
    void insert(const std::string& key, const std::string& value);
    //bool find(const std::string& key, std::string& value);
    
    uint64_t size() {return m_size;}
    uint64_t content_size();
    uint64_t bookkeeping_overhead() {return bookkeeping_overhead(MapT());}
    
    static std::string get_map_name() {return get_map_name(MapT());}
    static std::string get_map_name(const std::unordered_map<std::string, queue_t, HashT>& map)    {return "std";}
    static std::string get_map_name(const emhash7::HashMap<std::string, queue_t, HashT>& map)      {return "emh";}
    
private:
    const uint32_t CAPACITY;
        
    ListNode* m_queue;
    ListNode* m_front;
    ListNode* m_back;
    uint64_t m_size;
    
    MapT m_map;
    
private: // Trackers
    uint64_t t_hit;
    uint64_t t_search;
    
private:
    void detach(ListNode* node);
    void attach(ListNode* node);
    
    uint64_t bookkeeping_overhead(const std::unordered_map<std::string, queue_t, HashT>& map);
    uint64_t bookkeeping_overhead(const emhash7::HashMap<std::string, queue_t, HashT>& map);
};

#endif /* CacheLRU_hpp */
