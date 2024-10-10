/*
    本单元所有光影投射计算参考：https://lodev.org/cgtutor/raycasting.html#Performance
*/
#pragma once
#ifndef __CUDA_COMPILE__
    #include <span>
#endif

#ifdef __AVX__
    #include <immintrin.h>
#endif

#include <cmath>
#include <cstring>
#include <vector>
#include <iostream>
#include <string>
#include <iomanip>
#include "map2d.h"
#include "infrastructure.h"

#define ALIGNAS_SIZE 16

namespace camera {

constexpr int CANVAS_WIDTH  = 640;
constexpr int CANVAS_HEIGHT = 480;

using byte = uint8_t; 

#ifndef __CUDA_COMPILE__
    using map2d::Map2DView;
#endif

struct alignas(ALIGNAS_SIZE) Camera final{
public:
    explicit Camera();
    Camera(const std::string& str);
    ~Camera();

    Camera(const Camera& other)                = default;
    Camera& operator=(const Camera& other)     = default;
    Camera(Camera&& other) noexcept            = default;
    Camera& operator=(Camera&& other) noexcept = default;

#ifdef __CUDA_COMPILE__
    void Up(std::uint8_t* map, std::size_t width);
    void Down(std::uint8_t* map, std::size_t width);
    void Left(std::uint8_t* map, std::size_t width);
    void Right(std::uint8_t* map, std::size_t width);
#else
    void Up(const Map2DView map_view);
    void Down(const Map2DView map_view);
    void Left();
    void Right();

    void DrawAtCanvas(const Map2DView map_view, std::vector<byte>& canvas, 
                      tbb::affinity_partitioner& partitioner, tbb::task_arena& arena) const;
#endif

    /* 精度丢失: 4位小数截断。仅作哈希key使用 */
    std::string ConvertToHashKey() const;
    #pragma message("WARNING: std::string ConvertToHashKey() const has precision loss, truncated to 4 decimal places")
   
    operator std::string() const; 
    friend std::ostream& operator<<(std::ostream& os, const Camera& camera);

    [[deprecated("Variable coupling not yet removed")]]
    constexpr static std::size_t get_width();
    [[deprecated("Variable coupling not yet removed")]]
    constexpr static std::size_t get_height();

    float m_dir_x, m_dir_y;                              // 相机方向
    float m_plane_x, m_plane_y;                          // 相机平面法向量
    float m_position_x, m_position_y;                    // 相机位置

private:

    // FIXME: 去除硬编码
    constexpr static float rotSpeed  = 0.05f * 3.0f;     // 旋转速度  
    constexpr static float moveSpeed = 0.1f * 5.0f;      // 移动速度

    constexpr static std::uint8_t m_color_array[32] = {  // 颜色数组
        0,
        255,
        128,
        64, 
        200, 
        100,
    }; 

#ifndef __CUDA_COMPILE__
    /* 有GPU附加运算时使用 */
    void draw_at_canvas_fast(const Map2DView map_view, std::vector<byte>& canvas) const;
    /* 无GPU附加运算时使用 基测加速比约7.22~9.32 */
    void draw_at_canvas_faster(const Map2DView map_view, std::vector<byte>& canvas, 
                               tbb::affinity_partitioner& partitioner, tbb::task_arena& arena) const;
#endif                                                    
};

} // namespace camera