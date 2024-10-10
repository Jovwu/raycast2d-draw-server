#pragma once
#include <string>
#include <memory>
#include "camera.h"
#include "map2d.h"
#include "infrastructure.h"

namespace room {

using camera::Camera, camera::byte;
using map2d::Map2DBase, map2d::MapFactory;

class Room final {

public:
    Room();
    ~Room();
    Room(const Room& other)            = delete;
    Room& operator=(const Room& other) = delete;
    Room(Room&& other) noexcept;
    Room& operator=(Room&& other) noexcept;

    void CameraUp();
    void CameraDown();
    void CameraLeft();
    void CameraRight();
#ifndef __CUDA_COMPILE__
    map2d::Map2DView get_map_view();
#endif
    Camera get_camera();

private:
    Camera m_camera;          /*玩家相机*/
    Map2DBase* m_map;         /*玩家地图*/
};

} // namespace room