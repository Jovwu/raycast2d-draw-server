
#include <iostream>
#include <cstdint> 
#include <vector>
#include <iomanip>
#include <string>
#include <random>
#include <thread>
#include <cuda_runtime.h>
#include <thrust/universal_vector.h>
#include <tbb/tbb.h>
#include "camera.h"
#include "mredis.h"

namespace {

using camera::Camera;

std::shared_ptr<RedisSingleton> redis = nullptr;
tbb::task_group tg;

constexpr int PRE_STEP = 3;

using MyCudaFunc = void (camera::Camera::*)(std::uint8_t*, size_t);
MyCudaFunc funcs[] = {&Camera::Up, &Camera::Down, &Camera::Left, &Camera::Right};

std::vector<std::uint8_t> initial_map = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1,
        1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1,
        1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};
thrust::universal_vector<std::uint8_t> map(initial_map.begin(), initial_map.end());

void init(); 
void message_handler(const std::string& message);
__global__ void kernel(Camera camera, std::uint8_t* map, std::uint8_t* canvas);

void init() {
    
    redis = RedisSingleton::GetSingleton();
    redis->Set("test_redis_connect", "success");
    auto val = redis->Get("test_redis_connect");

    if (val)    printf("init redis success");
    else        printf("init redis failed");
    
    tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, tbb::this_task_arena::max_concurrency());
} 

void message_handler(const std::string& message) {
    
    tg.run([=](){

        Camera msg_camera(message);

        tbb::concurrent_vector<Camera> tasks;
        tasks.reserve(std::size(funcs) * PRE_STEP);

        std::uint8_t* map_ptr = thrust::raw_pointer_cast(initial_map.data());

        tbb::parallel_for(tbb::blocked_range<size_t>(0, std::size(funcs)), [&](const tbb::blocked_range<size_t>& range) {
            for(size_t i = range.begin(); i < range.end(); i++) {
                Camera task_camera = msg_camera;
                for(size_t j = 0; j < PRE_STEP; j++) {
                    (task_camera.*funcs[i])(map_ptr, 24);
                    tasks.emplace_back(task_camera);
                }
            }
        });

        tbb::parallel_for_each(tasks.begin(), tasks.end(), [](Camera& camera) {

            thrust::universal_vector<std::uint8_t> canvas(camera::CANVAS_WIDTH * camera::CANVAS_HEIGHT, 0);
            std::uint8_t* canvas_ptr = thrust::raw_pointer_cast(canvas.data());
            std::uint8_t* map_ptr = thrust::raw_pointer_cast(map.data());
            kernel<<<camera::CANVAS_HEIGHT, camera::CANVAS_WIDTH>>>(camera, map_ptr, canvas_ptr);
            cudaDeviceSynchronize();

            std::vector<std::uint8_t> std_canvas(canvas.size());
            thrust::copy(canvas.begin(), canvas.end(), std_canvas.begin());
            redis->Set(camera.ConvertToHashKey(), redis->CovertVectorToValue(std_canvas));
           
        });
    });
  
}

__global__ void kernel(Camera camera, std::uint8_t* map, std::uint8_t* canvas) {

    int x = threadIdx.x;
    int y = blockIdx.x;
 
    int map_x = static_cast<int>(camera.m_position_x), 
        map_y = static_cast<int>(camera.m_position_y),
        // 射线步进长度
        step_x, step_y, 
        side, hit = 0;

    float camera_x = 2 * x / static_cast<float>(camera::CANVAS_WIDTH) - 1,
        // 射线在 X 方向上迈出一步时在 y 方向上行进的单位数
        ray_dir_x = __fmaf_rn(camera.m_plane_x, camera_x, camera.m_dir_x),
        ray_dir_y = __fmaf_rn(camera.m_plane_y, camera_x, camera.m_dir_y),
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
        side_dist_x = (camera.m_position_x - map_x) * delta_dist_x;
    } else {
        step_x      = 1;
        side_dist_x = (map_x + 1.0 - camera.m_position_x) * delta_dist_x;
    }
    if (ray_dir_y < 0) {
        step_y      = -1;
        side_dist_y = (camera.m_position_y - map_y) * delta_dist_y;
    } else {
        step_y      = 1;
        side_dist_y = (map_y + 1.0 - camera.m_position_y) * delta_dist_y;
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
        if (map[static_cast<int>(__fmaf_rn(map_x, map2d::MAP2D_WIDTH, map_y))] > 0) hit = 1;
    }

    if (side == 0) perp_wall_dist = (side_dist_x - delta_dist_x);
    else           perp_wall_dist = (side_dist_y - delta_dist_y);

    int draw_start, draw_end;
    int line_height = static_cast<int>(camera::CANVAS_HEIGHT / perp_wall_dist);
    if (draw_start = (-line_height + camera::CANVAS_HEIGHT) >> 1, draw_start < 0)                  
        draw_start = 0;
    if (draw_end = (line_height + camera::CANVAS_HEIGHT) >> 1, draw_end >= camera::CANVAS_HEIGHT)  
        draw_end = camera::CANVAS_HEIGHT - 1;

    std::uint8_t color = 255;
    switch(map[static_cast<int>(__fmaf_rn(map_x, map2d::MAP2D_WIDTH, map_y))]) {
        case 1:  color = 255;    break; 
        case 2:  color = 128;    break; 
        case 3:  color = 64;     break; 
        case 4:  color = 200;    break; 
        default: color = 100;    break; 
    }

    color >>= side; // side = [0, 1]
    // = if (side == 1) color = color >> 1;

    if (draw_start <= y && y < draw_end) {
        canvas[static_cast<int>(__fmaf_rn(y, camera::CANVAS_WIDTH, x))] = color;
    }
}

} // namespace

int main() {

    init();

    std::thread subscriber_thread([]() {
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            redis->Subscribe("server2cuda", message_handler);
        }
    });
    subscriber_thread.join();

    return 0;
}