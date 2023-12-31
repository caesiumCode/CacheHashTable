#ifndef CacheHashTable_hpp
#define CacheHashTable_hpp

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include "CacheBase.hpp"
#include "CacheBase.cpp"

struct Range
{
    std::size_t lower;
    std::size_t upper;
};

template<typename HashT = std::hash<std::string>>
class CacheHashTable : public CacheBase<HashT>
{
public:
    CacheHashTable(uint8_t log2_slots, uint32_t length);
    ~CacheHashTable();
    
    void display();
    void display_trackers(double time);
    
    void insert(const std::string& key, const std::string& value);
    bool find(const std::string& key, std::string& value);
    
    uint64_t size();
    uint64_t content_size();
    uint64_t bookkeeping_overhead();
    
private:
    const uint32_t SLOTS;
    const uint32_t LENGTH;
    const uint32_t SIZE;
    
    const uint64_t SLOT_MASK;
    const uint8_t  LOG_LENGTH;
    HashT m_hasher;
    
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
    void        pass_string(std::size_t& i);
    
    std::size_t read_length(std::size_t& i);
    void        write_length(std::size_t& i, std::size_t length);
    void        write_string(std::size_t& i, const std::string& str);
};


#endif /* CacheHashTable_hpp */
