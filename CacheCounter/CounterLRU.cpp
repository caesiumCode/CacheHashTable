#include "CounterLRU.hpp"

template<typename HashT, typename MapT>
CounterLRU<HashT, MapT>::CounterLRU(uint32_t capacity)
: CAPACITY(capacity)
{
    m_queue = new ListNode[CAPACITY];
    m_front = nullptr;
    m_back = nullptr;
    m_size = 0;
    for (std::size_t i = 0; i < CAPACITY; i++)
    {
        m_queue[i].previous = nullptr;
        m_queue[i].next     = nullptr;
    }
    
    m_map.clear();
    
    t_search = 0;
    t_hit = 0;
}

template<typename HashT, typename MapT>
CounterLRU<HashT, MapT>::~CounterLRU()
{
    delete [] m_queue;
}

template<typename HashT, typename MapT>
void CounterLRU<HashT, MapT>::display()
{
    
}

template<typename HashT, typename MapT>
void CounterLRU<HashT, MapT>::display_trackers(double time)
{
    std::cout << get_map_name() << ",";                     // model
    std::cout << CounterBase<HashT>::get_hash_name() << ",";// Hash
    std::cout << CAPACITY << ",";                           // model parameter
    std::cout << (double) time/t_search << ",";             // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";        // hitrate (%)
    std::cout << size() << ",";                             // # pairs
    std::cout << content_size() << ",";                     // content size (B)
    std::cout << bookkeeping_overhead();                    // overhead (B)
    std::cout << std::endl;
}

template<typename HashT, typename MapT>
void CounterLRU<HashT, MapT>::display_counters()
{
    const std::size_t ORDERED_SIZE = 256;
    std::array<std::pair<std::string, uint64_t>, ORDERED_SIZE> counters;
    
    for (const auto& kv : m_map)
    {
        uint64_t val = (kv.second)->value;
        for (std::size_t i = 0; i < ORDERED_SIZE; i++) if (val > counters[i].second)
        {
            std::shift_right(counters.begin() + i, counters.end(), 1);
            counters[i].first  = kv.first;
            counters[i].second = val;
            
            break;
        }
    }
    
    std::cout << get_map_name() << "," << CounterBase<HashT>::get_hash_name() << "," << CAPACITY << "," << content_size() + bookkeeping_overhead();
    for (std::size_t i = 0; i < ORDERED_SIZE; i++) std::cout << "," << counters[i].first << "," << counters[i].second;
    
    std::cout << std::endl;
}

template<typename HashT, typename MapT>
void CounterLRU<HashT, MapT>::increment(const std::string &key)
{
    auto it = m_map.find(key);
    bool hit = it != m_map.end();
    
    // Select a node
    ListNode* node;
    if (!hit)
    {
        if (m_size < CAPACITY)
        {
            node = &m_queue[m_size];
            m_size++;
        }
        else
        {
            node = m_back;
            m_map.erase(node->key);
        }
        
        m_map[key]  = node;
        node->key   = key;
        node->value = 1;
    }
    else 
    {
        node = it->second;
        node->value++;
    }
    
    // Move-to-front
    detach(node);
    attach(node);
    
    // Update trackers
    t_search++;
    t_hit += hit;
}

template<typename HashT, typename MapT>
void CounterLRU<HashT, MapT>::detach(ListNode *node)
{
    if (node == m_back)  m_back  = node->previous;
    if (node == m_front) m_front = node->next;
    
    if (node->previous) node->previous->next = node->next;
    if (node->next)     node->next->previous = node->previous;
}

template<typename HashT, typename MapT>
void CounterLRU<HashT, MapT>::attach(ListNode *node)
{
    node->previous  = nullptr;
    node->next      = m_front;
    
    if (m_front) m_front->previous = node;
    
    if (m_size == 1) m_back = node;
    
    m_front = node;
}


template<typename HashT, typename MapT>
uint64_t CounterLRU<HashT, MapT>::content_size()
{
    uint32_t s = 0;
    
    for (std::size_t i = 0; i < m_size; i++) s += m_queue[i].key.size() + 8;
    
    return s;
}

template<typename HashT, typename MapT>
uint64_t CounterLRU<HashT, MapT>::bookkeeping_overhead(const std::unordered_map<std::string, queue_t, HashT>& map)
{
    return 8 * m_map.bucket_count() + 8 * m_map.size() + CAPACITY * 8 * 3;
}

template<typename HashT, typename MapT>
uint64_t CounterLRU<HashT, MapT>::bookkeeping_overhead(const emhash7::HashMap<std::string, queue_t, HashT>& map)
{
    return m_map.AllocSize(m_map.bucket_count()) + CAPACITY * 8 * 3;
}
