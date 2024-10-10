#include "room.h"

namespace room {
Room::Room() {
    m_camera = Camera(); 
    m_map = MapFactory::CreateMap(MapFactory::MapType::MAP2D1);
}

Room::~Room() {
    delete m_map;
}

Room::Room(Room&& other) noexcept : m_camera(std::move(other.m_camera)), m_map(other.m_map) {
    other.m_map = nullptr;
}

Room& Room::operator=(Room&& other) noexcept {
    if (this != &other) {
        m_camera = std::move(other.m_camera);
        delete m_map;
        m_map = other.m_map;
        other.m_map = nullptr;
    }
    return *this;
}

void Room::CameraUp() {
    m_camera.Up(m_map->View());
}

void Room::CameraDown() {
    m_camera.Down(m_map->View());
}

void Room::CameraLeft() {
    m_camera.Left();
}

void Room::CameraRight() {
    m_camera.Right();
}

#ifndef __CUDA_COMPILE__
map2d::Map2DView Room::get_map_view() {
    return m_map->View();
}
#endif

Camera Room::get_camera() {
    return std::move(Camera(m_camera));
}

} // namespace room