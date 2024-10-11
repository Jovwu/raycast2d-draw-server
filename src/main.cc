#include <iostream>
#include <algorithm>
#include <thread>
#include <pthread.h>
#include <sched.h>
#include "worker.h"
#include "infrastructure.h"
#include "client.h"

int main() {
    
    infrastructure::Init();

    unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::pair<std::thread*, bool>> threads(num_threads, {nullptr, false});
    
    while(true){
        std::size_t i = 0;
        while(std::get<1>(threads[i])) {
            i = (i + 1) % threads.size();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        delete std::get<0>(threads[i]);

        std::get<0>(threads[i]) = new std::thread(
            [](std::pair<std::thread*, bool>& thread_info) {
                std::get<1>(thread_info) = true;
                try {
                    Worker<client::Map2DClient> worker;
                    worker.Run();
                } catch(const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }
                std::get<1>(thread_info) = false;
            }, std::ref(threads[i])
        );
        infrastructure::SetAffinityAndPriority(*std::get<0>(threads[i]), i % num_threads, 
                                                    sched_get_priority_max(SCHED_FIFO));
    }

    return 0;
}