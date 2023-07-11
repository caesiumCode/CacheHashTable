#ifndef Hash_h
#define Hash_h

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



#endif /* Hash_h */

/*
 template<typename HashT>
 uint32_t CacheHashTable<HashT>::xorhash(const char *word)
 {
     char c;
     uint32_t h = 73802;
     
     for( ; ( c=*word ) != '\0' ; word++ ) h ^= ( (h << 5) + c + (h >> 2) );
     
     return h & 0x7fffffff;
 }
 */
