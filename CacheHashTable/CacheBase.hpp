#ifndef CacheBase_hpp
#define CacheBase_hpp

#include <string>

class CacheBase
{
public:
    CacheBase() = default;
    virtual ~CacheBase() = default;
    
    virtual void display() = 0;
    virtual void display_trackers(double time) = 0;
    
    virtual void insert(const std::string& key, const std::string& value) = 0;
};

#endif /* CacheBase_hpp */
