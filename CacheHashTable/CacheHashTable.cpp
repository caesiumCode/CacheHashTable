#include "CacheHashTable.hpp"


CacheHashTable::CacheHashTable(uint8_t log2_slots, uint32_t length)
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
    std::cout << (double) time/t_search << ",";         // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";    // hitrate (%)
    std::cout << (double) SIZE;                         // size (B)
    std::cout << std::endl;
}



void CacheHashTable::insert(const std::string &key, const std::string &value)
{
    // Update trackers
    t_search++;
    
    // Check plausibilty
    std::size_t new_span = ((key.size() >> 8) + 1) + key.size() + ((value.size() >> 8) + 1) + value.size();
    
    if (new_span <= LENGTH)
    {
        KEY_HASH    = m_hash_fun(key) & SLOT_MASK;
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


bool CacheHashTable::compare_string(std::size_t& i, const std::string &key)
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


void CacheHashTable::pass_string(std::size_t &i)
{
    i += read_length(i);
}



std::size_t CacheHashTable::read_length(std::size_t &i)
{
    std::size_t len = m_table[i];
    while (m_table[i] == 255 && i < END-1) len += m_table[++i];
    ++i;
    
    return len;
}



void CacheHashTable::write_length(std::size_t &i, std::size_t length)
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



void CacheHashTable::write_string(std::size_t &i, const std::string &str)
{
    std::size_t s = str.size();
    
    write_length(i, s);
    std::copy(reinterpret_cast<const uint8_t*>(str.data()), reinterpret_cast<const uint8_t*>(str.data() + s), m_table+i);
    
    i += s;
}
