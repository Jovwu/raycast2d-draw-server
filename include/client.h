#pragma once
#include <utility>
#include <typeinfo>
#include <any>
#include <mutex>
#include "message.pb.h"
#include "App.h"
#include "infrastructure.h"
#include "user.h"
#include "room.h"
#include "concurrentqueue.h"

namespace client {
    
using room::Room, camera::Camera, user::User;

namespace {

template <typename T>
concept clientmsgtype = std::is_same_v<T, Map2DClientMessage> 
                     || std::is_same_v<T, OtherClientMessage> 
                     || std::is_same_v<T, Map2DServerMessage> 
                     || std::is_same_v<T, OtherServerMessage>;

/* UwsDataBase */
template <typename Derived>
class UwsDataBase {
public:
    UwsDataBase() = default;
    virtual ~UwsDataBase() = default;
    
    UwsDataBase(const UwsDataBase& other)                = default;
    UwsDataBase& operator=(const UwsDataBase& other)     = default;
    UwsDataBase(UwsDataBase&& other) noexcept            = default;
    UwsDataBase& operator=(UwsDataBase&& other) noexcept = default;
    
    virtual std::string get_user_uuid() = 0;
    virtual std::pair<bool, std::any> Execute(std::any&& any) {
        spdlog::warn("Derived {} has not implemented this function: {}", typeid(Derived).name(), __func__);
        return {false, "execute failed"};
    };

    void set_uws(uWS::WebSocket<false, true, Derived>* uWS_t) {this->uWS_t = *uWS_t;}

    uWS::WebSocket<false, true, Derived>* get_uws() {return &uWS_t;};

protected:    
    uWS::WebSocket<false, true, Derived> uWS_t;
 
    template <clientmsgtype returntype>
    returntype Convert(std::any message){
        returntype result;
        try {
            result = std::any_cast<returntype>(message);
        } catch (const std::bad_any_cast& e) {
            spdlog::error("bad any cast: {}", e.what());
        }
        return std::move(result);
    }
};

} // namespace

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"

/* OtherClient */
class OtherClient final : public UwsDataBase<OtherClient> {
    std::string get_user_uuid() override {return "";};
};

/* Map2DClient */
class Map2DClient final : public UwsDataBase<Map2DClient> {
public:
    Map2DClient();
    ~Map2DClient() override;
    
    Map2DClient(const Map2DClient& other)                = delete;
    Map2DClient& operator=(const Map2DClient& other)     = delete;
    Map2DClient(Map2DClient&& other) noexcept            = default;
    Map2DClient& operator=(Map2DClient&& other) noexcept = default;

    std::string get_user_uuid() override;
    std::pair<bool, std::any> Execute(std::any&& any) override;
   
private:
    User m_user;
    Room m_room;

    std::unique_ptr<std::mutex> m_mtx;                          // std::mutex无法移动，只能用unique_ptr包装

    std::unique_ptr<tbb::task_arena>           m_arena;         // 并行任务区域: 规避任务窃取导致的上下文转移，cache-miss激增
    std::unique_ptr<tbb::affinity_partitioner> m_partitioner;   // 并行分区器: 考虑到画面的上下帧相似
    
};

#pragma GCC diagnostic pop

Map2DClient::Map2DClient() : m_user(User()), m_room(Room()), 
                             m_mtx(std::make_unique<std::mutex>()),
                             m_partitioner(std::make_unique<tbb::affinity_partitioner>()), 
                             m_arena(std::make_unique<tbb::task_arena>()) {}

Map2DClient::~Map2DClient() {}

std::string Map2DClient::get_user_uuid() {
    return m_user.get_uuid();
}

std::pair<bool, std::any> Map2DClient::Execute(std::any&& any) {

    Map2DClientMessage clientMessage = Convert<Map2DClientMessage>(any);
    Camera new_camera;

    // TODO:
    // - [x] 分离canvas的从属关系和计算逻辑 
    // - [ ] 锁定前后帧的强关联顺序(时间戳)
    // - [ ] 优化map从属关系，提升data-cache-hit
    {   
        // 糟糕的锁
        std::lock_guard<std::mutex> lock(*m_mtx);
        switch (clientMessage.ctl_flag()) {
            case Map2DClientMessage_ControlFlag::Map2DClientMessage_ControlFlag_UP:
                m_room.CameraUp();
                break;
            case Map2DClientMessage_ControlFlag::Map2DClientMessage_ControlFlag_DOWN:
                m_room.CameraDown();
                break;
            case Map2DClientMessage_ControlFlag::Map2DClientMessage_ControlFlag_LEFT:
                m_room.CameraLeft();
                break;
            case Map2DClientMessage_ControlFlag::Map2DClientMessage_ControlFlag_RIGHT: 
                m_room.CameraRight();
                break;
            default:
                spdlog::error("client ctl_flag unknown", m_user.get_uuid());
                return {false, std::string("client ctl_flag unknown")};
        }   

        new_camera = m_room.get_camera();
    }

    auto redis        = RedisSingleton::GetSingleton();
    std::string key   = new_camera.ConvertToHashKey();
    std::string value = redis->Get(key).value_or("");
    
    Map2DServerMessage serverMessage;
    if (value.empty()) {
        spdlog::warn("redis key not found");

        std::vector<camera::byte> canvas = std::vector<camera::byte>(camera::CANVAS_WIDTH * camera::CANVAS_HEIGHT, 0);
        new_camera.DrawAtCanvas(m_room.get_map_view(),canvas, *m_partitioner, *m_arena);
        std::string bytes_data(reinterpret_cast<const char*>(canvas.data()), canvas.size());
        serverMessage.set_canvas(bytes_data);

        (void)std::async(std::launch::async,
            [bytes_data = std::move(bytes_data), camera = std::move(new_camera), redis, key]() {
                redis->Publish("server2cuda", camera);
                redis->Set(key, bytes_data);
            }
        );
            
    } else {        
        spdlog::info("redis key found");   
        serverMessage.set_canvas(value);
    }

    return {true, std::move(serverMessage)};
};

} // namespace client