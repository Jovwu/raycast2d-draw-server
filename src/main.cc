#include <iostream>
#include <thread>
#include <algorithm>
#include <pthread.h>
#include <sched.h>
#include "worker.h"
#include "infrastructure.h"
#include "client.h"

void set_thread_affinity_and_priority(std::thread& t, int core_id, int priority) {

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
}

int main() {
    
    infrastructure::Init();

    unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread *> threads(num_threads);
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads[i] = new std::thread([i]() {
            Worker<client::Map2DClient> worker;
            worker.Run();
        });
        set_thread_affinity_and_priority(*threads[i], i % num_threads, 
                                         sched_get_priority_max(SCHED_FIFO));
    }

    std::for_each(threads.begin(), threads.end(), [](std::thread *t) {
        t->join();
    });
    
    return 0;
}