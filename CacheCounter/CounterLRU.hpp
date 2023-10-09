#ifndef CounterLRU_hpp
#define CounterLRU_hpp

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <utility>
#include <iomanip>

#include "CounterBase.hpp"
#include "CounterBase.cpp"

struct ListNode
{
    std::string key;
    uint64_t value;
    
    ListNode* previous;
    ListNode* next;
};

using queue_t = ListNode*;

template<typename HashT = std::hash<std::string>, typename MapT = std::unordered_map<std::string, queue_t, HashT>>
class CounterLRU : public CounterBase<HashT>
{
public:
    CounterLRU(uint32_t capacity);
    ~CounterLRU();
    
    void display();
    void display_trackers(double time);
    void display_counters();
    
    void increment(const std::string& key);
    
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

#endif /* CounterLRU_hpp */
