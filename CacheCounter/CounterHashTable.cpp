#include "CounterHashTable.hpp"

template<typename HashT>
CounterHashTable<HashT>::CounterHashTable(uint8_t log2_slots, uint32_t length)
: SLOTS(1 << log2_slots)
, SIZE(SLOTS * LENGTH)
, SLOT_MASK(SLOTS - 1)
, LOG_LENGTH(std::round(std::log2(length)))
, LENGTH(1 << uint8_t(std::round(std::log2(length))))
, RNG_STATE(0)
{
    m_table = new uint8_t[SIZE];
    for (std::size_t i = 0; i < SIZE; i++) m_table[i] = 0;
    
    RNG_MASK = new uint64_t[64];
    RNG_MASK[0] = 0;
    for (std::size_t i = 1; i < 64; i++) RNG_MASK[i] = 2 * RNG_MASK[i-1] + 1;
    
    t_hit    = 0;
    t_search = 0;
}

template<typename HashT>
CounterHashTable<HashT>::~CounterHashTable()
{
    delete [] m_table;
    delete [] RNG_MASK;
}

template<typename HashT>
void CounterHashTable<HashT>::display()
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
void CounterHashTable<HashT>::display_trackers(double time)
{
    std::cout << "fht,";                                    // model
    std::cout << CounterBase<HashT>::get_hash_name() << ",";  // Hash
    std::cout << SLOTS << "x" << LENGTH << ",";             // model parameter
    std::cout << (double) time/t_search << ",";             // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";        // hitrate (%)
    std::cout << size() << ",";                             // # pairs
    std::cout << content_size() << ",";                     // content size (B)
    std::cout << bookkeeping_overhead();                    // overhead (B)
    std::cout << std::endl << std::endl;
    
    for (std::size_t i = 0; i < SLOTS; i++)
    {
        std::size_t j = i*LENGTH;
        
        while (j < (i+1)*LENGTH && j < i*LENGTH + 64)
        {
            uint8_t len = m_table[j];

            std::cout << (int)len << " ";
            j++;
            std::size_t j_start = j;
            while (j < j_start+len && j < (i+1)*LENGTH)
            {
                std::cout << (char)m_table[j] << "_";
                j++;
            }
            if (j < (i+1)*LENGTH)
            {
                std::cout << (int)m_table[j] << " ";
                j++;
            }
        }
        
        std::cout << std::endl;
    }
}


template<typename HashT>
uint64_t CounterHashTable<HashT>::rng()
{
    uint64_t z = (RNG_STATE += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

template<typename HashT>
void CounterHashTable<HashT>::increment(const std::string &key)
{
    // Update trackers
    t_search++;
    
    // Check plausibilty
    std::size_t new_span = ((key.size() >> 8) + 1) + key.size() + 1;
    
    if (new_span <= LENGTH)
    {
        KEY_HASH    = m_hasher(key) & SLOT_MASK;
        START       = KEY_HASH << LOG_LENGTH;
        END         = START + LENGTH;
        
        Range range;
        bool        found       = find_loc(key, range);
        std::size_t span        = found * (range.upper - range.lower);
        uint8_t     log_counter = found * m_table[range.upper-1];
        
        // Shift everything in the slot
        std::size_t end = found ? range.upper : END;
        
        std::shift_right(m_table + START, m_table + end, span);
        if      (new_span > span) std::shift_right(m_table + START, m_table + END, new_span - span);
        else if (new_span < span) std::shift_left (m_table + START, m_table + END, span - new_span);
        
        // Insert at the beginning of the slot
        std::size_t i = START;
        write_string(i, key);
        uint8_t new_counter = found && (rng() & RNG_MASK[log_counter]) == 0 ? log_counter+1 : log_counter;
        m_table[i] = new_counter;
        
        // Update trackers
        t_hit += found;
    }
}


template<typename HashT>
bool CounterHashTable<HashT>::find_loc(const std::string &key, Range& range)
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
bool CounterHashTable<HashT>::compare_string(std::size_t& i, const std::string &key)
{
    // 3 D O T 4 4 M A N Y ...
    // ^
    
    // - - - - - MATCH KEY - - - - -
    // Read length
    std::size_t len = m_table[i];
    i++;
    
    // 3 D O T 4 4 M A N Y ...
    //   ^
    
    // Read string
    const uint8_t* key_translation = reinterpret_cast<const uint8_t*>(key.data());
    if (!std::equal(m_table + i, m_table + i + len, key_translation, key_translation + key.size()))
    {
        // Cannot match so pass data
        i += len;
        return false;
    }
    
    i += len + 1;
    
    // 3 D O T 4 4 M A N Y ...
    //           ^
    
    return i < END;
}


template<typename HashT>
void CounterHashTable<HashT>::write_string(std::size_t &i, const std::string &str)
{
    std::size_t s = str.size();
    
    m_table[i] = s;
    std::copy(reinterpret_cast<const uint8_t*>(str.data()), reinterpret_cast<const uint8_t*>(str.data() + s), m_table+i+1);
    
    i += 1 + s;
}

template<typename HashT>
uint64_t CounterHashTable<HashT>::bookkeeping_overhead()
{
    uint64_t s = 0;
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i = START;
        
        while (i < END)
        {
            std::size_t len = m_table[i];
            
            if (len > 0 && i + 1 + len + 1 <= END)
            {
                s += 2;
                i += len+2;
            }
            else
            {
                s += END - i;
                i = END;
            }
        }
    }
    
    return s;
}

template<typename HashT>
uint64_t CounterHashTable<HashT>::content_size()
{
    return SIZE - bookkeeping_overhead();
}

template<typename HashT>
uint64_t CounterHashTable<HashT>::size()
{
    uint64_t s = 0;
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i = START;
        
        while (i < END)
        {
            std::size_t len = m_table[i];
            
            if (len > 0 && i + 1 + len + 1 <= END)
            {
                s++;
                i += len+2;
            }
            else i = END;
        }
    }
    
    return s;
}
