#pragma once
#include <array>
#include <stdexcept>
#include <memory>

#ifndef __CUDA_COMPILE__
      #include <span>
#endif

#define ALIGNAS_SIZE 16

namespace map2d {

#ifndef __CUDA_COMPILE__
      using std::span;
      using Map2DView = std::span<int>;
#endif
    
constexpr int MAP2D_WIDTH  = 24;
constexpr int MAP2D_HEIGHT = 24;
using Map2DArray = std::array<int, MAP2D_WIDTH * MAP2D_HEIGHT>;

struct Map2DBase;

/* MapFactory */
class MapFactory {
public:
    enum class MapType {
        MAP2D1,
    };
    // TODO:
    // - [ ] 无需每次都创建新的Map2D对象，只需内存的一份引用即可
    static Map2DBase* CreateMap(MapType type);
};

 /* Map2DBase */
struct alignas(ALIGNAS_SIZE) Map2DBase {
friend class MapFactory;

public:
      Map2DBase();
      virtual ~Map2DBase() = default;
      Map2DBase(const Map2DBase& other);
      Map2DBase& operator=(const Map2DBase& other);
      Map2DBase(Map2DBase&& other) noexcept;
      Map2DBase& operator=(Map2DBase&& other) noexcept;

#ifndef __CUDA_COMPILE__
      Map2DView View();
#endif
      [[deprecated("Variable coupling not yet removed")]]
      constexpr static std::size_t get_width()  { return MAP2D_WIDTH; }
      [[deprecated("Variable coupling not yet removed")]]
      constexpr static std::size_t get_height() { return MAP2D_HEIGHT; }

protected:
      Map2DArray map;
      // FIXME: 去除硬编码
      float m_begin_position_x = 22;
      float m_begin_position_y = 12;

private:
      bool CheckMap() const;
};


/* Map2D-1 */
struct alignas(ALIGNAS_SIZE) Map2D1 final : public Map2DBase {
public:
      Map2D1();
      ~Map2D1() override;

      using Map2DBase::operator=;
      using Map2DBase::Map2DBase;
};

} // namespace map2d