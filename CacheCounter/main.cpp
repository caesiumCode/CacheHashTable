#include <fstream>
#include <unordered_map>
#include <chrono>

#include "CounterHashTable.hpp"
#include "CounterHashTable.cpp"

#include "CounterLRU.hpp"
#include "CounterLRU.cpp"


using Timer         = std::chrono::high_resolution_clock;
using TimerMeasure  = std::chrono::time_point<Timer>;

template<typename HashT>
void select_model(const char * argv[]);

template<typename HashT>
void test_model(CounterBase<HashT>& cache, const std::string& path, const std::string& filename, bool perf);

/*
 ./program (perf|track) <path> <dataset> <hash> fht(1|2|3) <log2_slots> <length>
 ./program (perf|track) <path> <dataset> <hash> (std|emh)  <capacity>
 */
int main(int argc, const char * argv[])
{
    std::string hash = std::string(argv[4]);
    
    if      (hash == "std") select_model<std::hash<std::string>>(argv);
    else if (hash == "xor") select_model<XorHash<73802>>(argv);
    else if (hash == "wyh") select_model<WyHash>(argv);
    else std::cout << "UNKNOWN HASH [" << hash << "]" << std::endl;
    
    return 0;
}

template<typename HashT>
void select_model(const char * argv[])
{
    std::string path     = std::string(argv[2]);
    std::string filename = std::string(argv[3]);
    std::string model    = std::string(argv[5]);
    
    if (model.substr(0, 3) == "fht")
    {
        uint8_t  log2_slots = std::atoi(argv[6]);
        uint32_t length     = std::atoi(argv[7]);
        
        int p = std::stoi(model.substr(3, 1));
        if (p == 1)
        {
            CounterHashTable<HashT, 1> ht(log2_slots, length);
            test_model<HashT>(ht, path, filename, std::string(argv[1]) == "perf");
        }
        else if (p == 2)
        {
            CounterHashTable<HashT, 2> ht(log2_slots, length);
            test_model<HashT>(ht, path, filename, std::string(argv[1]) == "perf");
        }
        else if (p == 4)
        {
            CounterHashTable<HashT, 4> ht(log2_slots, length);
            test_model<HashT>(ht, path, filename, std::string(argv[1]) == "perf");
        }
        else
        {
            CounterHashTable<HashT, 1> ht(log2_slots, length);
            test_model<HashT>(ht, path, filename, std::string(argv[1]) == "perf");
        }
    }
    else if (model == "std")
    {
        uint32_t capacity = std::atoi(argv[6]);
        
        CounterLRU<HashT, std::unordered_map<std::string, queue_t, HashT>> lru(capacity);
        test_model<HashT>(lru, path, filename, std::string(argv[1]) == "perf");
    }
    else if (model == "emh")
    {
        uint32_t capacity = std::atoi(argv[6]);
        
        CounterLRU<HashT, emhash7::HashMap<std::string, queue_t, HashT>> lru(capacity);
        test_model<HashT>(lru, path, filename, std::string(argv[1]) == "perf");
    }
    else std::cout << "UNKNOWN MODEL [" << model << "]" << std::endl;
}

template<typename HashT>
void test_model(CounterBase<HashT>& cache, const std::string& path, const std::string& filename, bool perf)
{
    const int   LINE_BUFFER_SIZE = 1 << 10;
    char        line_buffer[LINE_BUFFER_SIZE];
    std::FILE* fp = std::fopen((path + filename).c_str(), "r");
        
    
    if (perf)
    {
        TimerMeasure START = Timer::now();
        while (std::fgets(line_buffer, sizeof(line_buffer), fp))
        {
            std::string key(line_buffer);
            if (key.back() == '\n') key.pop_back();
            
            cache.increment(key);
        }
        TimerMeasure END = Timer::now();
        
        std::cout << filename << ",";
        cache.display_trackers(std::chrono::duration<double, std::nano>(END - START).count());
    }
    else
    {
        std::size_t chunk = 65536;
        if (filename == "scihub" || filename == "eclog" || filename == "accesslog") chunk = 4096;
        
        std::size_t pos = 1;
        while (std::fgets(line_buffer, sizeof(line_buffer), fp))
        {
            std::string key(line_buffer);
            if (key.back() == '\n') key.pop_back();
            
            cache.increment(key);
            
            if (true)
            {
                std::cout << filename << "," << pos << "," << key << std::endl;
                cache.display();
            }
            pos++;
        }
    }
    
        
    std::fclose(fp);
}
