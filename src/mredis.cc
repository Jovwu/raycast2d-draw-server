#include "mredis.h"

std::shared_ptr<RedisSingleton> RedisSingleton::m_instance = nullptr;
std::once_flag RedisSingleton::m_singleton_flag;
Redis RedisSingleton::m_redis = Redis("unix://password/var/run/redis/redis-server.sock");

std::shared_ptr<RedisSingleton> RedisSingleton::GetSingleton() {
    call_once(m_singleton_flag, 
        []() {
            m_instance.reset(new RedisSingleton("unix://zsy980326@/var/run/redis/redis-server.sock"));
        }
    );
    return m_instance;
}

void RedisSingleton::Set(const std::string& key, const std::string& value) {
    m_redis.set(key, value);
}

std::optional<std::string> RedisSingleton::Get(const std::string& key) {
    auto val = m_redis.get(key);
    if (val)   return *val;
    else       return std::nullopt;
}

void RedisSingleton::Command(const std::string &cmd) {
    m_redis.command(cmd);
}

void RedisSingleton::Del(const std::string& key) {
    m_redis.del(key);
}

std::string RedisSingleton::CovertVectorToValue(std::vector<std::uint8_t>& canvas) {
    std::string binary_value;
    binary_value.append(reinterpret_cast<char*>(canvas.data()), canvas.size());
    return binary_value;
}

void RedisSingleton::Publish(const std::string& channel, const std::string& message) {
    m_redis.publish(channel, message);
}

void RedisSingleton::Subscribe(const std::string& channel, std::function<void(const std::string&)> callback) {
    auto sub = m_redis.subscriber();
    sub.on_message(
        [&callback](std::string channel, std::string msg) {
            callback(msg);
        }
    );
    sub.subscribe(channel);
    while (true) sub.consume();
}