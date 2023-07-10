#include "SOA_CacheLRU.hpp"

SOA_CacheLRU::SOA_CacheLRU(uint32_t capacity)
: CAPACITY(capacity)
{
    m_queue.clear();
    m_map.clear();
    
    t_search = 0;
    t_hit = 0;
}

void SOA_CacheLRU::display()
{
    
}

void SOA_CacheLRU::display_trackers(double time)
{
    uint64_t total_size = 8 * m_map.bucket_count() + (8 + 8) * m_map.size() + m_queue.size() * 8 * 4 + get_content_size();
    
    std::cout << "blru,";                               // model
    std::cout << CAPACITY << ",";                       // model parameter
    std::cout << (double) time/t_search << ",";         // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";    // hitrate (%)
    std::cout << (double) total_size;                   // size (B)
    std::cout << std::endl;
}

void SOA_CacheLRU::insert(const std::string &key, const std::string &value)
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


void SOA_CacheLRU::clean()
{
    while(m_map.size() > CAPACITY)
    {
        m_map.erase(m_queue.rbegin()->first);
        m_queue.pop_back();
    }
};


uint64_t SOA_CacheLRU::get_content_size()
{
    uint32_t size = 0;
    
    for (const std::pair<std::string, std::string>& node : m_queue) size += node.first.size() + node.second.size();
    
    return size;
}
