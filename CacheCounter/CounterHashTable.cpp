#include "CounterHashTable.hpp"

template<typename HashT, uint8_t P>
CounterHashTable<HashT, P>::CounterHashTable(uint8_t log2_slots, uint32_t length)
: SLOTS(1 << log2_slots)
, SIZE(SLOTS * LENGTH)
, SLOT_MASK(SLOTS - 1)
, LOG_LENGTH(std::round(std::log2(length)))
, LENGTH(1 << uint8_t(std::round(std::log2(length))))
, RNG_STATE(12345678)
{
    m_table = new uint8_t[SIZE];
    for (std::size_t i = 0; i < SIZE; i++) m_table[i] = 0;
    
    U_MASK.resize(5);
    U_MASK[0] = 0;
    for (std::size_t i = 1; i < 5; i++)
    {
        U_MASK[i] = U_MASK[i-1];
        for (int j = 0; j < 16; j++) U_MASK[i] = 2 * U_MASK[i] + 1;
    }
    W_MASK.resize(64);
    W_MASK[0] = 0;
    for (std::size_t i = 1; i < 64; i++)
    {
        W_MASK[i] = 2 * W_MASK[i-1] + 1;
    }
    
    W.resize(64);
    for (std::size_t i = 0; i < 64; i++) W[i] = std::round(std::pow(2, (double)(P-1)/P * i));
    
    LOG_INV.resize(256);
    double a = std::pow(2, P);
    for (std::size_t i = 0; i < 64; i++) LOG_INV[i] = std::round((std::pow(a, i) - 1)/(a-1));
    
    t_hit    = 0;
    t_search = 0;
}

template<typename HashT, uint8_t P>
CounterHashTable<HashT, P>::~CounterHashTable()
{
    delete [] m_table;
}

template<typename HashT, uint8_t P>
void CounterHashTable<HashT, P>::display()
{
    for (uint16_t slot = 0; slot < SLOTS; slot++)
    {
        std::cout << "[SLOT" << std::setw(3) << slot << "]";
        
        bool is_char = false;
        int d = 0;
        for (std::size_t i = slot * LENGTH; i < (slot+1) * LENGTH; i++)
        {
            if (d == 0) is_char = false;
            
            if (is_char)
            {
                if (d > 1)  std::cout << std::setw(4) << reinterpret_cast<char&>(m_table[i]);
                else        std::cout << std::setw(4) << int(m_table[i]);
                d--;
            }
            else std::cout << std::setw(4) << int(m_table[i]);
            
            if (!is_char) d += m_table[i]+1;
            if (m_table[i] < 255) is_char = true;
        }
        
        std::cout << std::endl;
    }
}

template<typename HashT, uint8_t P>
void CounterHashTable<HashT, P>::display_trackers(double time)
{
    std::cout << "fht,";                                    // model
    std::cout << CounterBase<HashT>::get_hash_name() << ",";  // Hash
    std::cout << SLOTS << "x" << LENGTH << ",";             // model parameter
    std::cout << (double) time/t_search << ",";             // latency (ns)
    std::cout << (double) t_hit/t_search*100 << ",";        // hitrate (%)
    std::cout << size() << ",";                             // # pairs
    std::cout << content_size() << ",";                     // content size (B)
    std::cout << bookkeeping_overhead();                    // overhead (B)
    std::cout << std::endl;
}

template<typename HashT, uint8_t P>
void CounterHashTable<HashT, P>::display_counters()
{
    const std::size_t ORDERED_SIZE = 256;
    std::array<std::pair<std::string, uint64_t>, ORDERED_SIZE> counters{std::make_pair("", 0)};
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i   = START;
        
        while (i < END)
        {
            std::size_t len = m_table[i];
            
            if (len > 0 && i + 1 + len + 1 <= END)
            {
                uint64_t val = m_table[i + len + 1];
                for (std::size_t j = 0; j < ORDERED_SIZE; j++) if (val > counters[j].second)
                {
                    std::shift_right(counters.begin() + j, counters.end(), 1);
                    counters[j].first  = std::string(m_table+i+1, m_table+i+1+len);
                    counters[j].second = val;
                    
                    break;
                }
                
                i += len+2;
            }
            else i = END;
        }
    }
    
    std::cout << "fht" << int(P) << "," << CounterBase<HashT>::get_hash_name() << "," << SLOTS << "x" << LENGTH << "," << SIZE;
    for (std::size_t i = 0; i < ORDERED_SIZE; i++) std::cout << "," << counters[i].first << "," << LOG_INV[counters[i].second];
    
    std::cout << std::endl;
}


// splitmix64
template<typename HashT, uint8_t P>
uint64_t CounterHashTable<HashT, P>::rng()
{
    uint64_t z = (RNG_STATE += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

template<typename HashT, uint8_t P>
uint8_t CounterHashTable<HashT, P>::log_increment(uint8_t counter)
{
    if constexpr(P == 4)
    {
        return counter +
        (
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & W_MASK[counter & 63]) <= W[counter & 63])
         );
    }
    else if constexpr(P == 2)
    {
        return counter +
        (
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & W_MASK[counter & 63]) <= W[counter & 63])
         );
    }
    else return counter +
        (
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & U_MASK[counter >> 6]) == 0) &&
            ((rng() & W_MASK[counter & 63]) <= W[counter & 63])
         );
}

template<typename HashT, uint8_t P>
void CounterHashTable<HashT, P>::increment(const std::string &key)
{
    // Update trackers
    t_search++;
    
    // Check plausibilty
    std::size_t new_span = key.size() + 2;
    
    if (new_span <= LENGTH)
    {
        KEY_HASH    = m_hasher(key) & SLOT_MASK;
        START       = KEY_HASH << LOG_LENGTH;
        END         = START + LENGTH;
        
        Range range;
        bool        found       = find_loc(key, range);
        std::size_t span        = found * (range.upper - range.lower);
        uint8_t     log_counter = found ? m_table[range.upper-1] : 0;
        
        // Shift everything in the slot
        std::size_t end = found ? range.upper : END;
        
        std::shift_right(m_table + START, m_table + end, span);
        if      (new_span > span) std::shift_right(m_table + START, m_table + END, new_span - span);
        else if (new_span < span) std::shift_left (m_table + START, m_table + END, span - new_span);
        
        // Insert at the beginning of the slot
        std::size_t i = START;
        write_string(i, key);
        
        if (found) m_table[i] = log_increment(log_counter);
        else       m_table[i] = log_counter;
        
        // Update trackers
        t_hit += found;
    }
}


template<typename HashT, uint8_t P>
bool CounterHashTable<HashT, P>::find_loc(const std::string &key, Range& range)
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

template<typename HashT, uint8_t P>
bool CounterHashTable<HashT, P>::compare_string(std::size_t& i, const std::string &key)
{
    // 3 D O T 4 4 M A N Y ...
    // ^
    
    // - - - - - MATCH KEY - - - - -
    // Read length
    std::size_t len = m_table[i];
    i++;
    
    if (i + len >= SIZE)
    {
        i = END;
        return false;
    }
    
    // 3 D O T 4 4 M A N Y ...
    //   ^
    
    // Read string
    const uint8_t* key_translation = reinterpret_cast<const uint8_t*>(key.data());
    if (!std::equal(m_table + i, m_table + i + len, key_translation, key_translation + key.size()))
    {
        // Cannot match so pass data
        i += len + 1;
        return false;
    }
    
    i += len + 1;
    
    // 3 D O T 4 4 M A N Y ...
    //           ^
    
    return i < END;
}


template<typename HashT, uint8_t P>
void CounterHashTable<HashT, P>::write_string(std::size_t &i, const std::string &str)
{
    std::size_t s = str.size();
    
    m_table[i] = s;
    std::copy(reinterpret_cast<const uint8_t*>(str.data()), reinterpret_cast<const uint8_t*>(str.data() + s), m_table+i+1);
    
    i += 1 + s;
}

template<typename HashT, uint8_t P>
uint64_t CounterHashTable<HashT, P>::bookkeeping_overhead()
{
    uint64_t s = 0;
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i   = START;
        
        while (i < END)
        {
            std::size_t len = m_table[i];
            
            if (len > 0 && i + 1 + len + 1 <= END)
            {
                s++;
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

template<typename HashT, uint8_t P>
uint64_t CounterHashTable<HashT, P>::content_size()
{
    return SIZE - bookkeeping_overhead();
}

template<typename HashT, uint8_t P>
uint64_t CounterHashTable<HashT, P>::size()
{
    uint64_t s = 0;
    
    for (std::size_t START = 0; START < SIZE; START += LENGTH)
    {
        std::size_t END = START + LENGTH;
        std::size_t i   = START;
        
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
