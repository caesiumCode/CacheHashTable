#include "CacheHashTable.hpp"


CacheHashTable::CacheHashTable(uint8_t log2_slots, uint32_t length)
: SLOTS(1 << log2_slots)
, LENGTH(length)
, SIZE(SLOTS * LENGTH)
, SLOT_MASK(SLOTS - 1)
{
    m_table = new uint8_t[SIZE];
    for (std::size_t i = 0; i < SIZE; i++) m_table[i] = 0;
    
    t_hit    = 0;
    t_search = 0;    
}


CacheHashTable::~CacheHashTable()
{
    delete [] m_table;
}


void CacheHashTable::display()
{
    for (uint16_t slot = 0; slot < SLOTS; slot++)
    {
        std::cout << "[SLOT" << std::setw(3) << slot << "]";
        
        bool is_char = false;
        std::size_t d = 0;
        for (std::size_t i = slot * LENGTH; i < (slot+1) * LENGTH; i++)
        {
            if (d == 0) is_char = false;
            
            if (is_char)
            {
                std::cout << std::setw(4) << reinterpret_cast<char&>(m_table[i]);
                d--;
            }
            else std::cout << std::setw(4) << int(m_table[i]);
            
            if (!is_char) d += m_table[i];
            if (m_table[i] < 255) is_char = true;
        }
        
        std::cout << std::endl;
    }
}


void CacheHashTable::display_trackers(double time)
{
    std::cout << "fht,";                                // model
    std::cout << SLOTS << "x" << LENGTH << ",";         // model parameter
    std::cout << (double) time/t_search*1000000 << ","; // latency (μs)
    std::cout << (double) t_hit/t_search*100    << ","; // hitrate (%)
    std::cout << (double) SIZE;                         // size (B)
    std::cout << std::endl;
}



void CacheHashTable::insert(const std::string &key, const std::string &value)
{
    Range range;
    bool found = find_loc(key, range);
    std::size_t span = range.upper - range.lower;
    
    std::size_t key_hash = m_hash_fun(key) & SLOT_MASK;
    std::size_t start    = key_hash * LENGTH;
    std::size_t end      = found ? range.upper : start + LENGTH;
    
    // Shift everything in the slot
    std::size_t new_span = (key.size()/256 + 1) + key.size() + (value.size()/256 + 1) + value.size();
    std::size_t i;
    
    if (found)
    {
        for (i = end-1; i >= start + span; i--) m_table[i] = m_table[i - span];
        
        if (new_span > span)
        {
            std::size_t offset = new_span - span;
            for (i = start + LENGTH - 1; i >= start + offset; i--) m_table[i] = m_table[i - offset];
        }
        else if (new_span < span)
        {
            std::size_t offset = span - new_span;
            for (i = start; i < start + LENGTH - offset; i++) m_table[i] = m_table[i + offset];
        }
    }
    else for (i = end-1; i >= start + new_span; i--) m_table[i] = m_table[i - new_span];
    
    
    // Insert at the beginning of the slot
    i = start;
    write_string(i, key, start + LENGTH);
    write_string(i, value, start + LENGTH);
    
    // Update trackers
    t_search++;
    t_hit += found;
}



bool CacheHashTable::find(const std::string &key, std::string &value)
{
    Range range;
    
    // Not contained
    if (!find_loc(key, range)) return false;
    
    // Pass key
    std::size_t i = range.lower;
    pass_string(i);
    
    // Read data
    std::size_t len = read_length(i);
    
    value.resize(len);
    
    std::size_t j = 0;
    while (j < len)
    {
        value[j] = reinterpret_cast<char&>(m_table[i]);
        j++;
        i++;
    }
    
    return true;
}



bool CacheHashTable::find_loc(const std::string &key, Range& range)
{
    std::size_t key_hash = m_hash_fun(key) & SLOT_MASK;
    std::size_t start    = key_hash * LENGTH;
    std::size_t end      = start + LENGTH;
    std::size_t i        = start;
    std::size_t i_prev   = i;
    
    bool match = false;
    while (i < end && !match)
    {
        i_prev = i;
        match = compare_string(i, key, end);
    }
    
    if (match)
    {
        range.lower = i_prev;
        range.upper = i;
        return true;
    }
    else return false;
}


bool CacheHashTable::compare_string(std::size_t& i, const std::string &key, const std::size_t LIMIT)
{
    // 3 D O T 4 D A T A 4 M A N Y ...
    // ^
    
    // - - - - - MATCH KEY - - - - -
    // Read length
    std::size_t len = read_length(i, LIMIT);
    
    // 3 D O T 4 D A T A 4 M A N Y ...
    //   ^
    
    if (len != key.size() || i + len >= LIMIT)
    {
        // Cannot match so pass data
        i += len;
        pass_string(i, LIMIT);
        return false;
    }
    
    // Read string
    std::size_t j = 0;
    while (j < len && m_table[i] == reinterpret_cast<const uint8_t&>(key[j]))
    {
        i++;
        j++;
    }
    
    // 3 D O T 4 D A T A 4 M A N Y ...
    //         ^
    
    if (j < len)
    {
        // Don't match so pass data
        i += (len - j);
        pass_string(i, LIMIT);
        return false;
    }
    
    // - - - - - PASS DATA - - - - -
    pass_string(i, LIMIT);
    
    // 3 D O T 4 D A T A 4 M A N Y ...
    //                   ^
    
    if (i >= LIMIT) return false;
    
    return true;
}


void CacheHashTable::pass_string(std::size_t &i, const std::size_t LIMIT)
{
    i += read_length(i, LIMIT);
}



std::size_t CacheHashTable::read_length(std::size_t &i, const std::size_t LIMIT)
{
    std::size_t len = m_table[i];
    while (m_table[i] == 255 && i < LIMIT-1)
    {
        i++;
        len += m_table[i];
    }
    i++;
    
    return len;
}



void CacheHashTable::write_length(std::size_t &i, std::size_t length, const std::size_t LIMIT)
{
    while (length > 0 && i < LIMIT)
    {
        if (length >= 256) m_table[i] = 255;
        else               m_table[i] = length;
        
        length -= m_table[i];
        i++;
    }
    
    if (m_table[i-1] == 255 && i < LIMIT)
    {
        m_table[i] = 0;
        i++;
    }
}



void CacheHashTable::write_string(std::size_t &i, const std::string &str, const std::size_t LIMIT)
{
    write_length(i, str.size());
    
    std::size_t j = 0;
    while (j < str.size() && i < LIMIT)
    {
        m_table[i] = reinterpret_cast<const uint16_t&>(str[j]);
        i++;
        j++;
    }    
}
