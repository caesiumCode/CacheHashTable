#ifndef CounterBase_hpp
#define CounterBase_hpp

#include <string>
#include <unordered_map>

#include "Hash.hpp"
#include "Hash.cpp"

#include "emhash7.hpp"

template<typename HashT = std::hash<std::string>>
class CounterBase
{
public:
    CounterBase() = default;
    virtual ~CounterBase() = default;
    
    virtual void display() = 0;
    virtual void display_trackers(double time) = 0;
    
    virtual void increment(const std::string& key) = 0;
    
    virtual uint64_t size() = 0;
    virtual uint64_t content_size() = 0;
    virtual uint64_t bookkeeping_overhead() = 0;
    
    static std::string get_hash_name() {return get_hash_name(HashT());}
    
    template<uint32_t SEED>
    static std::string get_hash_name(const XorHash<SEED>& hash)             {return "xor" + std::to_string(SEED);}
    static std::string get_hash_name(const std::hash<std::string>& hash)    {return "std";}
    static std::string get_hash_name(const WyHash& hash)                    {return "wyh";}
};

#endif /* CounterBase_hpp */
