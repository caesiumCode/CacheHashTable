#include <fstream>
#include <unordered_map>
#include <chrono>

#include "CacheHashTable.hpp"
#include "CacheHashTable.cpp"

#include "CacheLRU.hpp"
#include "CacheLRU.cpp"


using Timer         = std::chrono::high_resolution_clock;
using TimerMeasure  = std::chrono::time_point<Timer>;

template<typename HashT>
void select_model(const char * argv[]);

template<typename HashT>
void test_model(CacheBase<HashT>& cache, const std::string& path, const std::string& filename);

/*
 ./program <path> <dataset> <hash> fht       <log2_slots> <length>
 ./program <path> <dataset> <hash> (std|emh) <capacity>
 */
int main(int argc, const char * argv[])
{
    std::string hash = std::string(argv[3]);
    
    if      (hash == "std")     select_model<std::hash<std::string>>(argv);
    else if (hash == "xor")     select_model<XorHash<335946>>(argv);
    //else if (hash == "xor")   select_model<XorHash<73802>>(argv);
    //else if (hash == "xor")   select_model<XorHash<335946>>(argv);
    //else if (hash == "xor")   select_model<XorHash<21500544>>(argv);
    else if (hash == "wyh")     select_model<WyHash>(argv);
    else std::cout << "UNKNOWN HASH [" << hash << "]" << std::endl;
    
    return 0;
}

template<typename HashT>
void select_model(const char * argv[])
{
    std::string path     = std::string(argv[1]);
    std::string filename = std::string(argv[2]);
    std::string model    = std::string(argv[4]);
    
    if (model == "fht")
    {
        uint8_t  log2_slots = std::atoi(argv[5]);
        uint32_t length     = std::atoi(argv[6]);
        
        CacheHashTable<HashT> ht(log2_slots, length);
        test_model<HashT>(ht, path, filename);
    }
    else if (model == "std")
    {
        uint32_t capacity = std::atoi(argv[5]);
        
        CacheLRU<HashT, std::unordered_map<std::string, queue_t, HashT>> lru(capacity);
        test_model<HashT>(lru, path, filename);
    }
    else if (model == "emh")
    {
        uint32_t capacity = std::atoi(argv[5]);
        
        CacheLRU<HashT, emhash7::HashMap<std::string, queue_t, HashT>> lru(capacity);
        test_model<HashT>(lru, path, filename);
    }
    else std::cout << "UNKNOWN MODEL [" << model << "]" << std::endl;
}

template<typename HashT>
void test_model(CacheBase<HashT>& cache, const std::string& path, const std::string& filename)
{
    const int   LINE_BUFFER_SIZE = 1 << 10;
    char        line_buffer[LINE_BUFFER_SIZE];
    std::FILE* fp = std::fopen((path + filename).c_str(), "r");
        
    TimerMeasure START = Timer::now();
    while (std::fgets(line_buffer, sizeof(line_buffer), fp))
    {
        std::string key(line_buffer);
        if (key.back() == '\n') key.pop_back();
        
        cache.insert(key, key);
    }
    TimerMeasure END = Timer::now();
        
    std::fclose(fp);
    
    std::cout << filename << ",";
    cache.display_trackers(std::chrono::duration<double, std::nano>(END - START).count());
}
