#include <fstream>
#include <unordered_map>
#include <chrono>

#include "CacheHashTable.hpp"
#include "CacheLRU.hpp"

using Timer         = std::chrono::high_resolution_clock;
using TimerMeasure  = std::chrono::time_point<Timer>;

void test_model(CacheBase& cache, const std::string& path, const std::string& filename);

/*
 ./program <path> <dataset> fht <log2_slots> <length>
 ./program <path> <dataset> lru <capacity>
 */
int main(int argc, const char * argv[])
{
    std::string path     = std::string(argv[1]);
    std::string filename = std::string(argv[2]);
    std::string model    = std::string(argv[3]);
    
    if (model == "fht")
    {
        uint8_t  log2_slots = std::atoi(argv[4]);
        uint32_t length     = std::atoi(argv[5]);
        
        CacheHashTable ht(log2_slots, length);
        test_model(ht, path, filename);
    }
    else if (model == "lru")
    {
        uint32_t capacity = std::atoi(argv[4]);
        
        CacheLRU lru(capacity);
        test_model(lru, path, filename);
    }
    else std::cout << "UNKNOWN MODEL [" << model << "]" << std::endl;
    
    return 0;
}

void test_model(CacheBase& cache, const std::string& path, const std::string& filename)
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
    cache.display_trackers(std::chrono::duration<double>(END - START).count());
}
