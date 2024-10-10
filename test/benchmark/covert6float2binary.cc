#include <benchmark/benchmark.h>
#include <string>
#include <cmath>
#include <immintrin.h>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(-1000.0, 1000.0);

static void normal_convert(benchmark::State& state)
{   
    for (auto _: state) {
        float truncated_digits[] = {dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen)};
        std::string binary_key(reinterpret_cast<char*>(truncated_digits), sizeof(truncated_digits));
    }
}
BENCHMARK(normal_convert);

static void simd_convert(benchmark::State& state)
{
     for (auto _: state) {
        __m256 avx_digits = _mm256_setr_ps(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), 0.0f, 0.0f);
        float truncated_digits[8];
        _mm256_storeu_ps(truncated_digits, avx_digits);
        std::string binary_key(reinterpret_cast<char*>(truncated_digits), sizeof(float) * 6);
    }
}
BENCHMARK(simd_convert);


static void normal_convert_truncate(benchmark::State& state)
{   
    for (auto _: state) {
        float truncated_digits[] = {std::trunc(dis(gen) * 1000) / 1000, 
                                    std::trunc(dis(gen) * 1000) / 1000, 
                                    std::trunc(dis(gen) * 1000) / 1000, 
                                    std::trunc(dis(gen) * 1000) / 1000,
                                    std::trunc(dis(gen) * 1000) / 1000,
                                    std::trunc(dis(gen) * 1000) / 1000};
        std::string binary_key(reinterpret_cast<char*>(truncated_digits), sizeof(truncated_digits));
    }
 
}
BENCHMARK(normal_convert_truncate);

static void simd_convert_truncate(benchmark::State& state)
{
     for (auto _: state) {
        __m256 float_values = _mm256_setr_ps(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), 0.0f, 0.0f);
        __m256 scaled_values = _mm256_mul_ps(float_values, _mm256_set1_ps(1000.0f));
        __m256 truncated_values = _mm256_round_ps(scaled_values, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
        __m256 result_values = _mm256_div_ps(truncated_values, _mm256_set1_ps(1000.0f));
        std::string binary_key(reinterpret_cast<char*>(&result_values), sizeof(result_values));
    }
}
BENCHMARK(simd_convert_truncate);


 
BENCHMARK_MAIN();