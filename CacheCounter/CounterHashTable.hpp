#ifndef CounterHashTable_hpp
#define CounterHashTable_hpp

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <random>

#include "CounterBase.hpp"
#include "CounterBase.cpp"

using dist_param_type = std::uniform_real_distribution<double>::param_type;

struct Range
{
    std::size_t lower;
    std::size_t upper;
};

template<typename HashT = std::hash<std::string>, uint8_t P = 4>
class CounterHashTable : public CounterBase<HashT>
{
public:
    CounterHashTable(uint8_t log2_slots, uint32_t length);
    ~CounterHashTable();
    
    void display();
    void display_trackers(double time);
    void display_counters();
    
    void increment(const std::string& key);
    
    uint64_t size();
    uint64_t content_size();
    uint64_t bookkeeping_overhead();
    
private:
    const uint32_t SLOTS;
    const uint32_t LENGTH;
    const uint32_t SIZE;
    
    const uint64_t SLOT_MASK;
    const uint8_t  LOG_LENGTH;
    HashT   m_hasher;
    
    std::vector<uint64_t> LOG_INV;
    std::vector<uint64_t> RNG_MASK;
    std::vector<uint64_t> RNG_RANGE;
    uint64_t RNG_CP_RANGE;
    uint64_t RNG_STATE;
    
    std::random_device rd;
    std::default_random_engine gen;
    std::uniform_real_distribution<double> uniform;
    
    uint8_t* m_table;
    
private: // Local (temporal) variable
    std::size_t KEY_HASH;
    std::size_t START;
    std::size_t END;
    
    
private: // Trackers
    uint64_t t_hit;
    uint64_t t_search;
    
private:
    bool        find_loc(const std::string& key, Range& range);
    bool        compare_string(std::size_t& i, const std::string& key);
    void        write_string(std::size_t& i, const std::string& str);
    
    uint64_t    rng();
    uint8_t     log_increment(uint8_t counter);
};

#endif /* CounterHashTable_hpp */
