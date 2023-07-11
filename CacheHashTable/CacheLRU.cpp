#include "CacheLRU.hpp"

template<typename HashT, typename MapT>
CacheLRU<HashT, MapT>::CacheLRU(uint32_t capacity)
: CAPACITY(capacity)
{
    m_queue.clear();
    m_map.clear();
    
    t_search = 0;
    t_hit = 0;
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
    
    if (hit)
    {
        m_queue.erase(it->second);
        m_queue.push_front(make_pair(key, value));
        
        it->second = m_queue.begin();
    }
    else
    {
        m_queue.push_front(make_pair(key, value));
        m_map.insert(make_pair(key, m_queue.begin()));
    }
    
    clean();
    
    // Update trackers
    t_search++;
    t_hit += (it != m_map.end());
}

template<typename HashT, typename MapT>
void CacheLRU<HashT, MapT>::clean()
{
    while(m_map.size() > CAPACITY)
    {
        m_map.erase(m_queue.rbegin()->first);
        m_queue.pop_back();
    }
};


template<typename HashT, typename MapT>
uint64_t CacheLRU<HashT, MapT>::content_size()
{
    uint32_t s = 0;
    
    for (const std::pair<std::string, std::string>& node : m_queue) s += node.first.size() + node.second.size();
    
    return s;
}

template<typename HashT, typename MapT>
uint64_t CacheLRU<HashT, MapT>::bookkeeping_overhead(const std::unordered_map<std::string, queue_it, HashT>& map)
{
    return 8 * m_map.bucket_count() + (8 + 8) * m_map.size() + m_queue.size() * 8 * 4;
}

template<typename HashT, typename MapT>
uint64_t CacheLRU<HashT, MapT>::bookkeeping_overhead(const emhash7::HashMap<std::string, queue_it, HashT>& map)
{
    return m_map.AllocSize(m_map.bucket_count()) + m_queue.size() * 8 * 4;
}
