#include <iostream>
#include <chrono>
#include <fstream>

using Timer         = std::chrono::high_resolution_clock;
using TimerMeasure  = std::chrono::time_point<Timer>;

/*
 ./program <path> <dataset>
 */

int main(int argc, const char * argv[])
{
    std::string path     = std::string(argv[1]);
    std::string filename = std::string(argv[2]);
    
    
    const int   LINE_BUFFER_SIZE = 1 << 10;
    char        line_buffer[LINE_BUFFER_SIZE];
    std::FILE* fp = std::fopen((path + filename).c_str(), "r");
        
    uint64_t n = 0;
    
    TimerMeasure START = Timer::now();
    while (std::fgets(line_buffer, sizeof(line_buffer), fp))
    {
        std::string key(line_buffer);
        if (key.back() == '\n') key.pop_back();
        
        ++n;
    }
    TimerMeasure END = Timer::now();
        
    std::fclose(fp);
    
    std::cout << filename << "," << n << "," << std::chrono::duration<double, std::nano>(END - START).count() << std::endl;
    
    return 0;
}
