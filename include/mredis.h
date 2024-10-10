#pragma once
#include <mutex>
#include <cmath>
#include <sw/redis++/redis++.h>

using sw::redis::Redis;

class RedisSingleton {
public:
    static std::shared_ptr<RedisSingleton> GetSingleton();

    void Set(const std::string& key, const std::string& value);
    std::optional<std::string> Get(const std::string& key);

    void Command(const std::string &cmd);
    void Del(const std::string& key);
    void Publish(const std::string& channel, const std::string& message);
    void Subscribe(const std::string& channel, std::function<void(const std::string&)> callback);

    std::string CovertVectorToValue(std::vector<std::uint8_t>& canvas);
    
private:
    explicit RedisSingleton(const std::string& url) {
        m_redis = Redis(url);
    }

    RedisSingleton(const RedisSingleton&)            = delete;
    RedisSingleton& operator=(const RedisSingleton&) = delete;
    RedisSingleton(RedisSingleton&&)                 = delete;
    RedisSingleton& operator=(RedisSingleton&&)      = delete;

    static std::shared_ptr<RedisSingleton> m_instance;
    static std::once_flag m_singleton_flag;
    static Redis m_redis;
};