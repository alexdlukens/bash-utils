#include <iostream>
#include <chrono>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
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
        auto time_t_time = std::chrono::system_clock::to_time_t(cur_time);
        
        
        char days_since[3];
        char hours_since[3];
        char min_since[3];
        char sec_since[3];
        std::snprintf(days_since, 3, "%01ld", (hhmmss.hours().count()/24)%100);
        std::snprintf(hours_since, 3, "%02ld", hhmmss.hours().count()%24);
        std::snprintf(min_since, 3, "%02ld", hhmmss.minutes().count()%60);
        std::snprintf(sec_since, 3, "%02ld", hhmmss.seconds().count()%60);

        auto cur_time_str = std::ctime(&time_t_time);
        cur_time_str = strtok(cur_time_str, "\n");

        std::cout << "\r" << cur_time_str << " | " \
                    << days_since << " days, " \
                    << hours_since << " hours, " \
                    << min_since << " minutes, " \
                    << sec_since << " seconds since starting";
        std::cout.flush();
    }
    
    std::cout << "\033[?47l\n";
    std::cout.flush();
    return 0;
}
