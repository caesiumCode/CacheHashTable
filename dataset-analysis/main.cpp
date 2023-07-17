#include <iostream>
#include <chrono>
#include <fstream>
#include <map>

using Timer         = std::chrono::high_resolution_clock;
using TimerMeasure  = std::chrono::time_point<Timer>;

/*
 ./program <path> <dataset>
 */

int main(int argc, const char * argv[])
{
    std::string path     = std::string(argv[1]);
    std::string filename = std::string(argv[2]);
    
    std::map<std::string, uint64_t> counter;
    
    const int   LINE_BUFFER_SIZE = 1 << 10;
    char        line_buffer[LINE_BUFFER_SIZE];
    std::FILE* fp = std::fopen((path + filename).c_str(), "r");
        
    uint64_t n = 0;
    
    TimerMeasure START = Timer::now();
    while (std::fgets(line_buffer, sizeof(line_buffer), fp))
    {
        std::string key(line_buffer);
        if (key.back() == '\n') key.pop_back();
        
        auto it = counter.find(key);
        if (it != counter.end()) it->second++;
        
        ++n;
    }
    TimerMeasure END = Timer::now();
        
    std::fclose(fp);
    
    //std::cout << filename << "," << n << "," << std::chrono::duration<double, std::nano>(END - START).count() << std::endl;
    std::cout << filename << ",";
    
    int i = 0;
    auto it = counter.begin();
    while (i <= 100)
    {
        std::cout << it->second << ",";
        it++;
    }
    
    
    return 0;
}
