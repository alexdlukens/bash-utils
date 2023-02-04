#include <iostream>
#include <chrono>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <csignal>

void sigintHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "\033[?47l\n";
    std::cout << "\033[u";
    std::cout.flush();

    exit(0);
}


int main()
{
    signal(SIGINT, sigintHandler);

    auto start_time = std::chrono::high_resolution_clock().now();
    std::cout << "\033[?47h";
    std::cout << "\033[s";
    std::cout.flush();
    system("clear");
    std::cout.flush();
    while(true)
    {
        sleep(1);
        auto cur_time = std::chrono::high_resolution_clock().now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(cur_time - start_time);
        auto hhmmss = std::chrono::hh_mm_ss(cur_time - start_time);
        std::cout << "\r";
        std::fprintf(stdout, "%02ld:%02ld", hhmmss.minutes().count()%60, hhmmss.seconds().count()%60);
        // std::cout << ":" << hhmmss.minutes().count() << ":" << hhmmss.seconds().count();
        std::cout.flush();
    }
    
    std::cout << "\033[?47l\n";
    std::cout.flush();
    return 0;
}
