#include <thread>
#include <future>
#include <tbb/tbb.h>
#include <App.h>
#include "message.pb.h"
#include "client.h"
#include "infrastructure.h"
#include "tools.h"

// TODO:
// - [ ] 使用配置文件写入
#define PORT 8080

using client::UwsDataBase, client::Map2DClient, client::OtherClient;

template <typename T1>
concept uwsdatatype = std::is_base_of_v<UwsDataBase<T1>, T1>;

template <typename T2>
concept prototype = std::is_same_v<T2, ClientMessage> || std::is_same_v<T2, ServerMessage>;

template <uwsdatatype T1>
class Worker final {
    using uWS_t = uWS::WebSocket<false, true, T1>;

public:
    Worker(){};
    ~Worker(){};
    Worker(const Worker& other)                = delete;
    Worker& operator=(const Worker& other)     = delete;
    Worker(Worker&& other) noexcept            = delete;
    Worker& operator=(Worker&& other) noexcept = delete;

    void Run();

private:
    template <prototype T2>
    struct FlowData final {
        FlowData() = default;
        FlowData(uWS_t* ws, T2 protoMessage){
            this->ws = ws;
            this->protoMessage = protoMessage;
        }
        FlowData(const FlowData& other) = default;
        FlowData& operator=(const FlowData& other) = default;
        FlowData(FlowData&& other) noexcept = default;
        FlowData& operator=(FlowData&& other) noexcept = default;
        uWS_t* ws;
        T2 protoMessage;
    };

    void uws_run();
    void uws_open(uWS_t *ws);
    inline void uws_message(uWS_t *ws, std::string_view message, uWS::OpCode opCode) [[gnu::always_inline]];
    inline void uws_send(uWS_t *ws, const bool result, const std::any& any) [[gnu::always_inline]];
    void uws_close(uWS_t *ws, int code, std::string_view message);
    
    // TODO:
    // - [ ] 返回值使用unique_ptr优化传递, 管理any所有权
    template <prototype T2>
    std::tuple<bool, std::any> serialize(std::string_view message_view);

    template <prototype T2>
    std::string deserialize(const bool result, const std::any& any);
};

template <uwsdatatype T1>
void Worker<T1>::Run() {
    uws_run();
}

template <uwsdatatype T1>
void Worker<T1>::uws_open(uWS_t *ws){

    T1* user_data = ws->getUserData();
    user_data->set_uws(ws);

#ifdef DEBUG
    spdlog::info("client {} join thread id: {}", user_data->get_user_uuid(), tool::GetThreadId());
#else
    spdlog::info("client {} join", user_data->get_user_uuid());
#endif

};

template <uwsdatatype T1>
void Worker<T1>::uws_message(uWS_t *ws, std::string_view message, uWS::OpCode opCode) {
#ifdef DEBUG
    tool::Time time(__func__);
#endif
    
    T1* user_data = ws->getUserData();

    try {
        // TODO:
        // - [ ] uws_send()使用RAII管理异步发送
        // - [ ] Execute()异步回传结果或者使用单独的执行队列
        if (opCode != uWS::OpCode::BINARY) {
            uws_send(ws, false, "client {} sent a non-binary request");
            return;
        }

        auto [serialize_is_done, serialize_result] = serialize<ClientMessage>(std::move(message));
        if (!serialize_is_done) {
            uws_send(ws, serialize_is_done, serialize_result);
            return;
        }

        auto [execute_is_done, execute_result] = user_data->Execute(std::move(serialize_result));
        uws_send(ws, execute_is_done, execute_result);

    } catch(const std::exception& e) {
        spdlog::error("client {} flow has exception {}", user_data->get_user_uuid(), e.what());
    }
    
}

template <uwsdatatype T1>
void Worker<T1>::uws_send(uWS_t *ws, const bool result, const std::any& any) {

    (void)std::async(std::launch::async,
        [this, ws, result, any = std::move(any)]() {
            std::string message = deserialize<ServerMessage>(result, std::move(any));
            if (!result) {
                spdlog::error("client {} flow has exception {}", ws->getUserData()->get_user_uuid(), any_cast<std::string>(any));
            }
            if (ws) {
                ws->send(message, uWS::OpCode::BINARY);
            }
        }
    );
}

template <uwsdatatype T1>
void Worker<T1>::uws_close(uWS_t *ws, int code, std::string_view message) {
    T1* user_data = ws->getUserData();
    spdlog::info("client {} leave", user_data->get_user_uuid());
};

template <uwsdatatype T1>
void Worker<T1>::uws_run() {

    uWS::App().ws<T1>("/*", {
        .compression            = uWS::DEDICATED_COMPRESSOR,
        .maxPayloadLength       = 512 * 1024,
        .idleTimeout            = 600,
        .resetIdleTimeoutOnSend = true,
        .sendPingsAutomatically = false,
        .open                   = [this](uWS_t *ws) { this->uws_open(ws);},
        .message                = [this](uWS_t *ws, std::string_view message, uWS::OpCode opCode) 
                                  {this->uws_message(ws, message, opCode);},
        // .pong                = [](auto */*ws*/, std::string_view) {spdlog::info("pong");},
        // .drain               = [](auto *ws) {spdlog::info("Buffer drained, buffered amount: {}", ws->getBufferedAmount());}
        .close                  = [this](uWS_t *ws, int code, std::string_view message) 
                                  {this->uws_close(ws, code, message);}  
    })  .listen(PORT, [this](auto *token) {
        // if (token)             {spdlog::info("Listening on port {}", PORT);} 
        // else                   {spdlog::error("Failed to listen on port {}", PORT);}
    })  .run();
}

template <uwsdatatype T1>
template <prototype T2>
std::tuple<bool, std::any> Worker<T1>::serialize(std::string_view message_view) {

    T2 message;

    [[unlikely]] if (!message.ParseFromArray(message_view.data(), message_view.size())) { 
        return {false, std::string("serialize message failed")};
    } else {
        if constexpr (std::is_same_v<T1, Map2DClient>) {
            return {true, std::move(message.for_map2d())};
        } else if constexpr (std::is_same_v<T1, OtherClient>) {
            return {true, std::move(message.for_other())};
        } else {
            return {false, std::string("the serialized type is not registered")};
        }
    }
}

template <uwsdatatype T1>
template <prototype T2>
std::string Worker<T1>::deserialize(const bool result, const std::any& any) { 

    T2 message;
    std::string deserialize_msg;
    std::string error_msg;

    [[unlikely]] if (!any.has_value() || result == false) {
        error_msg = any_cast<std::string>(any);
    } else {
        if constexpr (std::is_same_v<T1, Map2DClient> && std::is_same_v<T2, ServerMessage>) {
            message.mutable_for_map2d()->CopyFrom(any_cast<Map2DServerMessage>(any));
        } 
        else if constexpr (std::is_same_v<T1, OtherClient> && std::is_same_v<T2, ServerMessage>) {
            message.mutable_for_other()->CopyFrom(any_cast<OtherServerMessage>(any));
        } 
        else if constexpr (std::is_same_v<T1, Map2DClient> && std::is_same_v<T2, ClientMessage>) {
            message.mutable_for_map2d()->CopyFrom(any_cast<Map2DClientMessage>(any));
        } 
        else if constexpr (std::is_same_v<T1, OtherClient> && std::is_same_v<T2, ClientMessage>) {
            message.mutable_for_other()->CopyFrom(any_cast<OtherClientMessage>(any));
        }   
    }

    if constexpr (std::is_same_v<T2, ServerMessage>) {
        message.set_error_code(result ? 0 : 1);
        message.set_error_msg(result ? "success" : error_msg);
    } 
    else if constexpr (std::is_same_v<T2, ClientMessage>) {
        message.set_ctl_flag(result ? 0 : 1);
    }
    
    if (!message.SerializeToString(&deserialize_msg)) {
        spdlog::error("deserialize message failed");
        return "";
    }

    return std::move(deserialize_msg);
}