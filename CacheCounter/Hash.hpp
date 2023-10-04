#ifndef Hash_hpp
#define Hash_hpp

#include <string>
#include "wyhash.h"

template<uint32_t SEED>
struct XorHash
{
    uint32_t operator()(const std::string& key) const
    {
        uint32_t h = SEED;
        
        for (char c : key) h ^= ( (h << 5) + c + (h >> 2) );
                
        return h & 0x7fffffff;
    }
};


struct WyHash
{
    uint64_t operator()(const std::string& key) const
    {
        return wyhash(key.c_str(), key.size(), 0, _wyp);
    }
};

#endif /* Hash_hpp */
