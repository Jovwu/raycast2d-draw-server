#include "map2d.h"

namespace map2d {
/* MapFactory */
Map2DBase* MapFactory::CreateMap(MapType type) {
    Map2DBase* map;
    switch (type) {
        case MapType::MAP2D1:{ map = new Map2D1(); break; }  
        default: throw std::invalid_argument("Unknown Map type");
    }
    if (!map->CheckMap()) {
        throw std::invalid_argument("Invalid Map");
    } 
    return map;
}

/* Map2DBase */
Map2DBase::Map2DBase() {}

#ifndef __CUDA_COMPILE__
Map2DView Map2DBase::View() {
    return Map2DView(map.data(), map.size());
}
#endif

Map2DBase::Map2DBase(const Map2DBase& other) : map(other.map) {}

Map2DBase& Map2DBase::operator=(const Map2DBase& other) {
    if (this != &other) {
        map = other.map;
    }
    return *this;
}

Map2DBase::Map2DBase(Map2DBase&& other) noexcept : map(std::move(other.map)) {}

Map2DBase& Map2DBase::operator=(Map2DBase&& other) noexcept {
    if (this != &other) {
        map = std::move(other.map);
    }
    return *this;
}

bool Map2DBase::CheckMap() const {
    return MAP2D_WIDTH == 0 || MAP2D_HEIGHT == 0 || map.empty() || 
           map.size() != MAP2D_WIDTH * MAP2D_HEIGHT || 
           m_begin_position_x < 0 || m_begin_position_y < 0 || 
           m_begin_position_x >= MAP2D_WIDTH || m_begin_position_y >= MAP2D_HEIGHT 
           ? false : true;
}

/* Map2D-1 */
Map2D1::Map2D1() {
    map = {
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
}

Map2D1::~Map2D1() {}

} // namespace map2d