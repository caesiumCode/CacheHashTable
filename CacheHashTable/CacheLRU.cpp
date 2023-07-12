#include "CacheLRU.hpp"

template<typename HashT, typename MapT>
CacheLRU<HashT, MapT>::CacheLRU(uint32_t capacity)
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
CacheLRU<HashT, MapT>::~CacheLRU()
{
    delete [] m_queue;
}

template<typename HashT, typename MapT>
void CacheLRU<HashT, MapT>::display()
{
    
}

template<typename HashT, typename MapT>
void CacheLRU<HashT, MapT>::display_trackers(double time)
{
    std::cout << get_map_name() << ",";                     // model
    std::cout << CacheBase<HashT>::get_hash_name() << ",";  // Hash
    std::cout << CAPACITY << ",";                           // model parameter
    std::cout << (double) time/t_search << ",";             // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";        // hitrate (%)
    std::cout << size() << ",";                             // # pairs
    std::cout << content_size() << ",";                     // content size (B)
    std::cout << bookkeeping_overhead();                    // overhead (B)
    std::cout << std::endl;
}

template<typename HashT, typename MapT>
void CacheLRU<HashT, MapT>::insert(const std::string &key, const std::string &value)
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
        node->value = value;
    }
    else node = it->second;
    
    // Move-to-front
    detach(node);
    attach(node);
    
    // Update trackers
    t_search++;
    t_hit += hit;
}

template<typename HashT, typename MapT>
void CacheLRU<HashT, MapT>::detach(ListNode *node)
{
    if (node == m_back)  m_back  = node->previous;
    if (node == m_front) m_front = node->next;
    
    if (node->previous) node->previous->next = node->next;
    if (node->next)     node->next->previous = node->previous;
}

template<typename HashT, typename MapT>
void CacheLRU<HashT, MapT>::attach(ListNode *node)
{
    node->previous  = nullptr;
    node->next      = m_front;
    
    if (m_front) m_front->previous = node;
    
    if (m_size == 1) m_back = node;
    
    m_front = node;
}


template<typename HashT, typename MapT>
uint64_t CacheLRU<HashT, MapT>::content_size()
{
    uint32_t s = 0;
    
    for (std::size_t i = 0; i < m_size; i++) s += m_queue[i].key.size() + m_queue[i].value.size();
    
    return s;
}

template<typename HashT, typename MapT>
uint64_t CacheLRU<HashT, MapT>::bookkeeping_overhead(const std::unordered_map<std::string, queue_t, HashT>& map)
{
    return 8 * m_map.bucket_count() + (8 + 8) * m_map.size() + CAPACITY * 8 * 4;
}

template<typename HashT, typename MapT>
uint64_t CacheLRU<HashT, MapT>::bookkeeping_overhead(const emhash7::HashMap<std::string, queue_t, HashT>& map)
{
    return m_map.AllocSize(m_map.bucket_count()) + CAPACITY * 8 * 4;
}
