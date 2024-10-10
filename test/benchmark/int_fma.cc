#include <benchmark/benchmark.h>
#include <immintrin.h>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, INT32_MAX);

static void normal_fma(benchmark::State& state)
{   
    for (auto _: state) {
        int result = dis(gen) * dis(gen) + dis(gen);
    }
}
BENCHMARK(normal_fma);

static void std_fma(benchmark::State& state)
{   
    for (auto _: state) {
        int result = std::fma(dis(gen), dis(gen), dis(gen));
    }
 
}
BENCHMARK(std_fma);
 
BENCHMARK_MAIN();