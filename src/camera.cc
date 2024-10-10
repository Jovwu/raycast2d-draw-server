#include "camera.h"
namespace camera {

Camera::Camera() {
    this->m_position_x = 22.0f;
    this->m_position_y = 12.0f;
    this->m_dir_x      = -1.0f;
    this->m_dir_y      = 0.0f;
    this->m_plane_x    = 0.0f;
    this->m_plane_y    = 0.66f;
}

Camera::Camera(const std::string& str) {
    float* float_values = reinterpret_cast<float*>(const_cast<char*>(str.data()));
    this->m_position_x  = float_values[0];
    this->m_position_y  = float_values[1];
    this->m_dir_x       = float_values[2];
    this->m_dir_y       = float_values[3];
    this->m_plane_x     = float_values[4];
    this->m_plane_y     = float_values[5];
}

Camera::~Camera() {}

#ifdef __CUDA_COMPILE__
void Camera::Up(std::uint8_t* map, std::size_t width) {
    if (map[static_cast<int>(std::fma(static_cast<int>(std::fma(this->m_dir_x, moveSpeed, this->m_position_x)), width, static_cast<int>(this->m_position_y)))] == false)
    // = if (map[int(this->position_x - this->dir_x * moveSpeed) * width + int(this->position_y)] == false)
        this->m_position_x = std::fma(this->m_dir_x, moveSpeed, this->m_position_x);
    if (map[static_cast<int>(std::fma(static_cast<int>(this->m_position_x), width, static_cast<int>(std::fma(this->m_dir_y, moveSpeed, this->m_position_y))))] == false)
    // = if (map[int(this->position_x) * width + int(this->position_y + this->dir_y * moveSpeed)] == false)
        this->m_position_y = std::fma(this->m_dir_y, moveSpeed, this->m_position_y);
}

void Camera::Down(std::uint8_t* map, std::size_t width) {
    if (map[static_cast<int>(std::fma(static_cast<int>(this->m_position_x - this->m_dir_x * moveSpeed), width, static_cast<int>(this->m_position_y)))] == false)
    // = if (map[int(this->position_x - this->dir_x * moveSpeed) * width + int(this->position_y)] == false)
        this->m_position_x -= this->m_dir_x * moveSpeed;
    if (map[static_cast<int>(std::fma(static_cast<int>(this->m_position_x), width, static_cast<int>(this->m_position_y - this->m_dir_y * moveSpeed)))] == false)
    // = if (map[int(this->position_x) * width + int(this->position_y - this->dir_y * moveSpeed)] == false)
        this->m_position_y -= this->m_dir_y * moveSpeed;
}

void Camera::Right(std::uint8_t* map, std::size_t width) {
    float old_m_dir_x   = this->m_dir_x;
    this->m_dir_x       = this->m_dir_x * cosf(-rotSpeed) - this->m_dir_y * sinf(-rotSpeed);
    this->m_dir_y       = std::fma(old_m_dir_x, sinf(-rotSpeed), this->m_dir_y * cosf(-rotSpeed));
    float old_m_plane_x = this->m_plane_x;
    this->m_plane_x     = this->m_plane_x * cosf(-rotSpeed) - this->m_plane_y * sinf(-rotSpeed);
    this->m_plane_y     = std::fma(old_m_plane_x, sinf(-rotSpeed), this->m_plane_y * cosf(-rotSpeed));
}

void Camera::Left(std::uint8_t* map, std::size_t width) {
    float old_m_dir_x   = this->m_dir_x;
    this->m_dir_x       = this->m_dir_x * cosf(rotSpeed) - this->m_dir_y * sinf(rotSpeed);
    this->m_dir_y       = std::fma(old_m_dir_x, sinf(rotSpeed), this->m_dir_y * cosf(rotSpeed));
    float old_m_plane_x = this->m_plane_x;
    this->m_plane_x     = this->m_plane_x * cosf(rotSpeed) - this->m_plane_y * sinf(rotSpeed);
    this->m_plane_y     = std::fma(old_m_plane_x, sinf(rotSpeed), this->m_plane_y * cosf(rotSpeed));
}

#else

void Camera::DrawAtCanvas(const Map2DView map_view, std::vector<byte>& canvas, 
                          tbb::affinity_partitioner& partitioner, tbb::task_arena& arena) const {
    // 平衡IO和计算耗时
    if (infrastructure::IsGpuAvailable()) draw_at_canvas_fast(map_view, canvas);
    else                                  draw_at_canvas_faster(map_view, canvas, partitioner, arena);
}

void Camera::Up(const Map2DView map_view) {
    std::size_t width = map2d::MAP2D_WIDTH;
    if (map_view[static_cast<int>(std::fma(static_cast<int>(std::fma(this->m_dir_x, moveSpeed, this->m_position_x)), width, static_cast<int>(this->m_position_y)))] == false)
    // = if (map[int(this->position_x - this->dir_x * moveSpeed) * width + int(this->position_y)] == false)   
        this->m_position_x = std::fma(this->m_dir_x, moveSpeed, this->m_position_x);
    if (map_view[static_cast<int>(std::fma(static_cast<int>(this->m_position_x), width, static_cast<int>(std::fma(this->m_dir_y, moveSpeed, this->m_position_y))))] == false)
    // = if (map[int(this->position_x) * width + int(this->position_y + this->dir_y * moveSpeed)] == false)   
        this->m_position_y = std::fma(this->m_dir_y, moveSpeed, this->m_position_y);
}

void Camera::Down(const Map2DView map_view) {
    std::size_t width = map2d::MAP2D_WIDTH;
    if (map_view[static_cast<int>(std::fma(static_cast<int>(this->m_position_x - this->m_dir_x * moveSpeed), width, static_cast<int>(this->m_position_y)))] == false)
    // = if (map[int(this->position_x - this->dir_x * moveSpeed) * width + int(this->position_y)] == false)
        this->m_position_x -= this->m_dir_x * moveSpeed;    
    if (map_view[static_cast<int>(std::fma(static_cast<int>(this->m_position_x), width, static_cast<int>(this->m_position_y - this->m_dir_y * moveSpeed)))] == false)
    // = if (map[int(this->position_x) * width + int(this->position_y - this->dir_y * moveSpeed)] == false)
        this->m_position_y -= this->m_dir_y * moveSpeed;
}

void Camera::Right() {
    float old_m_dir_x   = this->m_dir_x;
    this->m_dir_x       = this->m_dir_x * cosf(-rotSpeed) - this->m_dir_y * sinf(-rotSpeed);
    this->m_dir_y       = std::fma(old_m_dir_x, sinf(-rotSpeed), this->m_dir_y * cosf(-rotSpeed));
    float old_m_plane_x = this->m_plane_x;
    this->m_plane_x     = this->m_plane_x * cosf(-rotSpeed) - this->m_plane_y * sinf(-rotSpeed);
    this->m_plane_y     = std::fma(old_m_plane_x, sinf(-rotSpeed), this->m_plane_y * cosf(-rotSpeed));
}

void Camera::Left() {
    float old_m_dir_x   = this->m_dir_x;
    this->m_dir_x       = this->m_dir_x * cosf(rotSpeed) - this->m_dir_y * sinf(rotSpeed);
    this->m_dir_y       = old_m_dir_x * sinf(rotSpeed) + this->m_dir_y * cosf(rotSpeed);
    float old_m_plane_x = this->m_plane_x;
    this->m_plane_x     = this->m_plane_x * cosf(rotSpeed) - this->m_plane_y * sinf(rotSpeed);
    this->m_plane_y     = old_m_plane_x * sinf(rotSpeed) + this->m_plane_y * cosf(rotSpeed);
}
#endif

constexpr std::size_t Camera::get_width() {
    return CANVAS_WIDTH;
}

constexpr std::size_t Camera::get_height() {
    return CANVAS_HEIGHT;
}

#ifdef __AVX__
std::string Camera::ConvertToHashKey() const {
    __m256 float_values     = _mm256_setr_ps(m_position_x, m_position_y, m_dir_x, m_dir_y, m_plane_x, m_plane_y, 0.0f, 0.0f);
    __m256 scaled_values    = _mm256_mul_ps(float_values, _mm256_set1_ps(1000.0f));
    __m256 truncated_values = _mm256_round_ps(scaled_values, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
    __m256 result_values    = _mm256_div_ps(truncated_values, _mm256_set1_ps(1000.0f));
    return std::string(reinterpret_cast<char*>(&result_values), sizeof(result_values));
}
#else
std::string Camera::ConvertToHashKey() const {
    float truncated_digits[] = {trunc(m_position_x * 1000) / 1000, 
                                trunc(m_position_y * 1000) / 1000, 
                                trunc(m_dir_x      * 1000) / 1000, 
                                trunc(m_dir_y      * 1000) / 1000,
                                trunc(m_plane_x    * 1000) / 1000,
                                trunc(m_plane_y    * 1000) / 1000};
    return std::string(reinterpret_cast<char*>(truncated_digits), sizeof(truncated_digits));
}
#endif

#ifdef __AVX__
Camera::operator std::string() const {
    __m256 avx_digits = _mm256_setr_ps(m_position_x, m_position_y, m_dir_x, m_dir_y, m_plane_x, m_plane_y, 0.0f, 0.0f);
    float truncated_digits[8];
    _mm256_storeu_ps(truncated_digits, avx_digits);
    return std::string(reinterpret_cast<char*>(truncated_digits), sizeof(float) * 6);
}
#else
Camera::operator std::string() const {
    float truncated_digits[] = {m_position_x, m_position_y, m_dir_x, m_dir_y, m_plane_x, m_plane_y};
    return std::string(reinterpret_cast<char*>(truncated_digits), sizeof(truncated_digits));
}
#endif

std::ostream& operator<<(std::ostream& os, const Camera& camera) {
    os << std::fixed << std::setprecision(8);
    os << "Camera(" 
       << "m_position_x: " <<  camera.m_position_x  << ", "
       << "m_position_y: " <<  camera.m_position_y  << ", "
       << "m_dir_x: "      <<  camera.m_dir_x       << ", "
       << "m_dir_y: "      <<  camera.m_dir_y       << ", "
       << "m_plane_x: "    <<  camera.m_plane_x     << ", "
       << "m_plane_y: "    <<  camera.m_plane_y     << ")";
    return os;
}

#ifndef __CUDA_COMPILE__

void Camera::draw_at_canvas_fast(const Map2DView map_view, std::vector<byte>& canvas) const {

#ifdef DEBUG
    tool::Time time(__func__);
#endif

    for (std::size_t x = 0; x < CANVAS_WIDTH; x++) {

        int map_x = static_cast<int>(m_position_x), 
            map_y = static_cast<int>(m_position_y),
            // 射线步进长度
            step_x, step_y, 
            side, hit = 0;

        float camera_x = 2 * x / static_cast<float>(CANVAS_WIDTH) - 1,
              // 射线在 X 方向上迈出一步时在 y 方向上行进的单位数
              ray_dir_x = std::fma(this->m_plane_x, camera_x, m_dir_x),
              ray_dir_y = std::fma(this->m_plane_y, camera_x, m_dir_y),
              // 射线从 1 个 x 边到下一个 x 边或从 1 个 y 边到下一个 y 边必须行进的距离
              delta_dist_x = (ray_dir_x == 0) ? 1e30 : abs(1 / ray_dir_x),
              delta_dist_y = (ray_dir_y == 0) ? 1e30 : abs(1 / ray_dir_y),
              // 射线从其起始位置到第一个 x 边和第一个 y 边必须行进的距离
              side_dist_x, side_dist_y,
              // 射线的长度 
              perp_wall_dist;

        // 根据射线方向决定步进和初始距离
        if (ray_dir_x < 0) {
            step_x      = -1;
            side_dist_x = (m_position_x - map_x) * delta_dist_x;
        } else {
            step_x      = 1;
            side_dist_x = (map_x + 1.0 - m_position_x) * delta_dist_x;
        }
        if (ray_dir_y < 0) {
            step_y      = -1;
            side_dist_y = (m_position_y - map_y) * delta_dist_y;
        } else {
            step_y      = 1;
            side_dist_y = (map_y + 1.0 - m_position_y) * delta_dist_y;
        }

        // DDA: 检测射线撞墙
        while (hit == 0) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x       += step_x;
                side        = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y       += step_y;
                side        = 1;
            }
            if (map_view[static_cast<int>(std::fma(map_x, map2d::MAP2D_WIDTH, map_y))] > 0) hit = 1;
        }

        if (side == 0) perp_wall_dist = (side_dist_x - delta_dist_x);
        else           perp_wall_dist = (side_dist_y - delta_dist_y);

        int draw_start, draw_end;
        int line_height = static_cast<int>(CANVAS_HEIGHT / perp_wall_dist);
        if (draw_start = (-line_height + CANVAS_HEIGHT) >> 1, draw_start < 0)          draw_start = 0;
        if (draw_end = (line_height + CANVAS_HEIGHT) >> 1, draw_end >= CANVAS_HEIGHT)  draw_end   = CANVAS_HEIGHT - 1;

        std::uint8_t color = 255;
        color = m_color_array[map_view[static_cast<int>(std::fma(map_x, map2d::MAP2D_WIDTH, map_y))]];
        color >>= side; // side = [0, 1]
        // = if (side == 1) color = color >> 1;

        for(std::size_t y = draw_start; y < draw_end; ++y) {
            canvas[static_cast<int>(std::fma(y, CANVAS_WIDTH, x))] = color;
        }
    }
}

void Camera::draw_at_canvas_faster(const Map2DView map_view, std::vector<byte>& canvas, 
                                   tbb::affinity_partitioner& partitioner, tbb::task_arena& arena) const {
#ifdef DEBUG
    tool::Time time(__func__);
#endif

    arena.execute(
        [&] {
            // TODO:
            // - [ ] 优化partitioner
            // - [ ] 矩阵转置(但需要两个partitioner)
            tbb::parallel_for((std::size_t)0, (std::size_t)CANVAS_WIDTH, 
                [&] (std::size_t x) {

                    int map_x = static_cast<int>(m_position_x), 
                        map_y = static_cast<int>(m_position_y),
                        // 射线步进长度
                        step_x, step_y, 
                        side, hit = 0;

                    float camera_x = 2 * x / static_cast<float>(CANVAS_WIDTH) - 1,
                          // 射线在 X 方向上迈出一步时在 y 方向上行进的单位数
                          ray_dir_x = std::fma(this->m_plane_x, camera_x, m_dir_x),
                          ray_dir_y = std::fma(this->m_plane_y, camera_x, m_dir_y),
                          // 射线从 1 个 x 边到下一个 x 边或从 1 个 y 边到下一个 y 边必须行进的距离
                          delta_dist_x = (ray_dir_x == 0) ? 1e30 : abs(1 / ray_dir_x),
                          delta_dist_y = (ray_dir_y == 0) ? 1e30 : abs(1 / ray_dir_y),
                          // 射线从其起始位置到第一个 x 边和第一个 y 边必须行进的距离
                          side_dist_x, side_dist_y,
                          // 射线的长度 
                          perp_wall_dist;

                    // 根据射线方向决定步进和初始距离
                    if (ray_dir_x < 0) {
                        step_x      = -1;
                        side_dist_x = (m_position_x - map_x) * delta_dist_x;
                    } else {
                        step_x      = 1;
                        side_dist_x = (map_x + 1.0 - m_position_x) * delta_dist_x;
                    }
                    if (ray_dir_y < 0) {
                        step_y      = -1;
                        side_dist_y = (m_position_y - map_y) * delta_dist_y;
                    } else {
                        step_y      = 1;
                        side_dist_y = (map_y + 1.0 - m_position_y) * delta_dist_y;
                    }

                    // DDA: 检测射线撞墙
                    while (hit == 0) {
                        if (side_dist_x < side_dist_y) {
                            side_dist_x += delta_dist_x;
                            map_x       += step_x;
                            side        = 0;
                        } else {
                            side_dist_y += delta_dist_y;
                            map_y       += step_y;
                            side        = 1;
                        }
                        if (map_view[static_cast<int>(std::fma(map_x, map2d::MAP2D_WIDTH, map_y))] > 0) hit = 1;
                    }

                    if (side == 0) perp_wall_dist = (side_dist_x - delta_dist_x);
                    else           perp_wall_dist = (side_dist_y - delta_dist_y);

                    int draw_start, draw_end;
                    int line_height = static_cast<int>(CANVAS_HEIGHT / perp_wall_dist);
                    if (draw_start = (-line_height + CANVAS_HEIGHT) >> 1, draw_start < 0)          draw_start = 0;
                    if (draw_end = (line_height + CANVAS_HEIGHT) >> 1, draw_end >= CANVAS_HEIGHT)  draw_end   = CANVAS_HEIGHT - 1;

                    std::uint8_t color = 255;
                    color = m_color_array[map_view[static_cast<int>(std::fma(map_x, map2d::MAP2D_WIDTH, map_y))]];
                    color >>= side; // side = [0, 1]
                    // = if (side == 1) color = color >> 1;

                    for(std::size_t y = draw_start; y < draw_end; ++y) {
                        canvas[static_cast<int>(std::fma(y, CANVAS_WIDTH, x))] = color;
                    }
                // NOTE: partitioner理应由矩阵类型定义
                }, partitioner
           );
        }
    );
}

#endif

} // namespace camera