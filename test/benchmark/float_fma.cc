#include <benchmark/benchmark.h>
#include <immintrin.h>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(-1000.0, 1000.0);

static void normal_fma(benchmark::State& state)
{   
    for (auto _: state) {
        for (size_t i = 0; i < 4; i++) {
            float result = dis(gen) * dis(gen) + dis(gen);
        }     
    }
}
BENCHMARK(normal_fma);

static void simd_fma(benchmark::State& state)
{
     for (auto _: state) {
        __m128 float_values = _mm_setr_ps(dis(gen), dis(gen), dis(gen), dis(gen));
        __m128 result_values = _mm_fmadd_ps(float_values, float_values, _mm_set1_ps(dis(gen)));
    }
}
BENCHMARK(simd_fma);


static void std_fma(benchmark::State& state)
{   
    for (auto _: state) {
        for (size_t i = 0; i < 4; i++) { 
            float result = std::fma(dis(gen), dis(gen), dis(gen));
        }
    }
 
}
BENCHMARK(std_fma);
 
BENCHMARK_MAIN();