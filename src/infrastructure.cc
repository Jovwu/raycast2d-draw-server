#include "infrastructure.h"
namespace infrastructure {

namespace {

void log_init() {
#ifndef __CUDA_COMPILE__
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("[%^%l%$] %v");
        
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
        file_sink->set_level(spdlog::level::info);

        spdlog::sinks_init_list sink_list = { file_sink, console_sink };
        
        spdlog::set_default_logger(std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list({console_sink, file_sink})));

    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
#endif
}

void tbb_init() {
    tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, tbb::this_task_arena::max_concurrency());
    tbb::task_scheduler_init init;
}

void redis_init() {

    auto redis = RedisSingleton::GetSingleton();

    redis->Set("test_redis_connect", "success");

    auto val = redis->Get("test_redis_connect");
    if (val)    spdlog::info("init redis: {}", *val);
    else        spdlog::error("init redis failed");
    
    redis->Command("FLUSHALL");
}

void gpu_monitor() {
    std::thread(
        []() {
            while(true) {
                cuda_server_pid = tool::GetLocalPidByName("jovwucuda");
                if (cuda_server_pid == -1)  spdlog::error("cuda server not found");
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }
    ).detach(); 
}
    
} // namespace

void Init() {

    log_init();
    tbb_init();
    redis_init();

#ifdef DEBUG
    spdlog::info("debug mode");
#endif

#ifdef __AVX__
    spdlog::info("hello avx!");
#endif

    server_pid = tool::GetLocalPidByName("jovwuserver");
    spdlog::info("server_pid: {}", server_pid);

    gpu_monitor();
}

bool IsGpuAvailable() {
    return cuda_server_pid != -1;
}

void SetAffinityAndPriority(std::thread& t, int core_id, int priority) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        spdlog::error("Error calling pthread_setaffinity_np: {}", rc);
    }

    struct sched_param param;
    param.sched_priority = priority;
    rc = pthread_setschedparam(t.native_handle(), SCHED_FIFO, &param);
    if (rc != 0) {
        spdlog::error("Error calling pthread_setschedparam: {}", rc);
    }
};

} // namespace infrastructure
