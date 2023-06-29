#include "CacheLRU.hpp"

CacheLRU::CacheLRU(uint32_t capacity)
: CAPACITY(capacity)
{
    m_queue.clear();
    m_map.clear();
    
    t_search = 0;
    t_hit = 0;
}

void CacheLRU::display()
{
    
}

void CacheLRU::display_trackers(double time)
{
    uint64_t total_size = 8 * m_map.bucket_count() + (8 + 8) * m_map.size() + m_queue.size() * 8 * 4 + get_content_size();
    
    std::cout << "lru,";                                // model
    std::cout << CAPACITY << ",";                       // model parameter
    std::cout << (double) time/t_search*1000000 << ","; // latency (μs)
    std::cout << (double) t_hit/t_search*100    << ","; // hitrate (%)
    std::cout << (double) total_size;                   // size (B)
    std::cout << std::endl;
}

void CacheLRU::insert(const std::string &key, const std::string &value)
{
    auto it = m_map.find(key);
    
    if (it != m_map.end())
    {
        m_queue.erase(it->second);
        m_map.erase(it);
    }
    
    m_queue.push_front(make_pair(key, value));
    m_map.insert(make_pair(key, m_queue.begin()));
    
    clean();
    
    // Update trackers
    t_search++;
    t_hit += (it != m_map.end());
}


void CacheLRU::clean()
{
    while(m_map.size() > CAPACITY)
    {
        m_map.erase(m_queue.rbegin()->first);
        m_queue.pop_back();
    }
};


uint64_t CacheLRU::get_content_size()
{
    uint32_t size = 0;
    
    for (const std::pair<std::string, std::string>& node : m_queue) size += node.first.size() + node.second.size();
    
    return size;
}
