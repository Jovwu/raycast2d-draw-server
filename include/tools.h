#pragma once
#include <string>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <chrono> 
#include <thread>
#include <sstream>
#include <dirent.h>
#include <uuid/uuid.h>

#ifndef __CUDA_COMPILE__
    #include <spdlog/spdlog.h>
#endif

namespace tool {

using namespace std::chrono;

std::string GenerateUUID();
std::string GetThreadId();
int GetLocalPidByName(const char *process_name);

struct Time {

    Time(const std::string& function_name = "unknown function");
    ~Time();

private:
    high_resolution_clock::time_point m_start;
    high_resolution_clock::time_point m_end;
    std::string m_function_name;
};

} // namespace tool