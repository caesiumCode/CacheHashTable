#include "CacheHashTable.hpp"

template<typename HashT>
CacheHashTable<HashT>::CacheHashTable(uint8_t log2_slots, uint32_t length)
: SLOTS(1 << log2_slots)
, SIZE(SLOTS * LENGTH)
, SLOT_MASK(SLOTS - 1)
, LOG_LENGTH(std::round(std::log2(length)))
, LENGTH(1 << uint8_t(std::round(std::log2(length))))
{
    m_table = new uint8_t[SIZE];
    for (std::size_t i = 0; i < SIZE; i++) m_table[i] = 0;
    
    t_hit    = 0;
    t_search = 0;    
}

template<typename HashT>
CacheHashTable<HashT>::~CacheHashTable()
{
    delete [] m_table;
}

template<typename HashT>
void CacheHashTable<HashT>::display()
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

template<typename HashT>
void CacheHashTable<HashT>::display_trackers(double time)
{
    std::cout << "fht,";                                    // model
    std::cout << CacheBase<HashT>::get_hash_name() << ",";  // Hash
    std::cout << SLOTS << "x" << LENGTH << ",";             // model parameter
    std::cout << (double) time/t_search << ",";             // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";        // hitrate (%)
    std::cout << size() << ",";                             // # pairs
    std::cout << content_size() << ",";                     // content size (B)
    std::cout << bookkeeping_overhead();                    // overhead (B)
    std::cout << std::endl;
}


template<typename HashT>
void CacheHashTable<HashT>::insert(const std::string &key, const std::string &value)
{
    // Update trackers
    t_search++;
    
    // Check plausibilty
    std::size_t new_span = ((key.size() >> 8) + 1) + key.size() + ((value.size() >> 8) + 1) + value.size();
    
    if (new_span <= LENGTH)
    {
        KEY_HASH    = m_hasher(key) & SLOT_MASK;
        START       = KEY_HASH << LOG_LENGTH;
        END         = START + LENGTH;
        
        Range range;
        bool found = find_loc(key, range);
        std::size_t span = found * (range.upper - range.lower);
        
        // Shift everything in the slot
        std::size_t end = found ? range.upper : END;
        
        std::shift_right(m_table + START, m_table + end, span);
        if      (new_span > span) std::shift_right(m_table + START, m_table + END, new_span - span);
        else if (new_span < span) std::shift_left(m_table + START, m_table + END, span - new_span);
        
        // Insert at the beginning of the slot
        std::size_t i = START;
        write_string(i, key);
        write_string(i, value);
        
        // Update trackers
        t_hit += found;
    }
}


template<typename HashT>
bool CacheHashTable<HashT>::find(const std::string &key, std::string &value)
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


template<typename HashT>
bool CacheHashTable<HashT>::find_loc(const std::string &key, Range& range)
{
    std::size_t i       = START;
    std::size_t i_prev  = i;
    
    bool match = false;
    while (i < END && !match)
    {
        i_prev = i;
        match = compare_string(i, key);
    }
    
    range.lower = i_prev;
    range.upper = i;
    
    return match;
}

template<typename HashT>
bool CacheHashTable<HashT>::compare_string(std::size_t& i, const std::string &key)
{
    // 3 D O T 4 D A T A 4 M A N Y ...
    // ^
    
    // - - - - - MATCH KEY - - - - -
    // Read length
    std::size_t len = read_length(i);
    
    // 3 D O T 4 D A T A 4 M A N Y ...
    //   ^
    
    // Read string
    const uint8_t* key_translation = reinterpret_cast<const uint8_t*>(key.data());
    if (!std::equal(m_table + i, m_table + i + len, key_translation, key_translation + key.size()))
    {
        // Cannot match so pass data
        i += len;
        pass_string(i);
        return false;
    }
    
    i += len;
    
    // 3 D O T 4 D A T A 4 M A N Y ...
    //         ^
    
    // - - - - - PASS DATA - - - - -
    pass_string(i);
    
    // 3 D O T 4 D A T A 4 M A N Y ...
    //                   ^
    
    return i < END;
}

template<typename HashT>
void CacheHashTable<HashT>::pass_string(std::size_t &i)
{
    i += read_length(i);
}


template<typename HashT>
std::size_t CacheHashTable<HashT>::read_length(std::size_t &i)
{
    std::size_t len = m_table[i];
    while (m_table[i] == 255 && i < END-1) len += m_table[++i];
    ++i;
    
    return len;
}


template<typename HashT>
void CacheHashTable<HashT>::write_length(std::size_t &i, std::size_t length)
{
    if (length >= 256)
    {
        while (length > 0 && i < END)
        {
            if (length >= 256) m_table[i] = 255;
            else               m_table[i] = length;
            
            length -= m_table[i++];
        }
        
        if (m_table[i-1] == 255 && i < END) m_table[i++] = 0;
    }
    else
    {
        m_table[i] = length;
        ++i;
    }
}


template<typename HashT>
void CacheHashTable<HashT>::write_string(std::size_t &i, const std::string &str)
{
    std::size_t s = str.size();
    
    write_length(i, s);
    std::copy(reinterpret_cast<const uint8_t*>(str.data()), reinterpret_cast<const uint8_t*>(str.data() + s), m_table+i);
    
    i += s;
}

template<typename HashT>
uint64_t CacheHashTable<HashT>::bookkeeping_overhead()
{
    uint64_t s = 0;
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i = START;
        
        while (i < END)
        {
            std::size_t i_prev = i;
            uint64_t    part_s = 0;
            bool        match  = false;
            
            // Read key
            std::size_t len = read_length(i);
            
            if (i < END && len > 0)
            {
                part_s += (len / 256) + 1;
                i += len;
                
                if (i < END)
                {
                    // Read data
                    len = read_length(i);
                    
                    part_s += (len / 256) + 1;
                    
                    if (i + len <= END)
                    {
                        s += part_s;
                        match = true;
                    }
                    
                    i += len;
                }
            }
            
            if (!match)
            {
                s += END - i_prev;
                i = END;
            }
        }
    }
    
    return s;
}

template<typename HashT>
uint64_t CacheHashTable<HashT>::content_size()
{
    return SIZE - bookkeeping_overhead();
}

template<typename HashT>
uint64_t CacheHashTable<HashT>::size()
{
    uint64_t s = 0;
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i = START;
        
        while (i < END)
        {
            bool match = false;
            
            // Read key
            std::size_t len = read_length(i);
            
            if (i < END && len > 0)
            {
                i += len;
                
                if (i < END)
                {
                    // Read data
                    len = read_length(i);

                    if (i + len <= END) match = true;
                    
                    i += len;
                }
            }
            
            if (!match) i = END;
            
            s += match;
        }
    }
    
    return s;
}
