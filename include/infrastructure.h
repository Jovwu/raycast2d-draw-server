#pragma once
#include <iostream>
#include <tbb/tbb.h>
#include "mredis.h"
#include "tools.h"

#ifndef __CUDA_COMPILE__
    #include "spdlog/spdlog.h"
    #include "spdlog/sinks/stdout_color_sinks.h"
    #include "spdlog/sinks/basic_file_sink.h"
#endif

namespace infrastructure {
    
namespace {

static int server_pid = -1,
           cuda_server_pid = -1;

void log_init();
void tbb_init();
void redis_init();
void gpu_monitor();

} // namespace

void Init();
bool IsGpuAvailable();

} // namespace infrastructure