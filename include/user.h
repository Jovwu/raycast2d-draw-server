#pragma once
#include "tools.h"

namespace user{

class User final {
public:
    User();
    ~User();
    User(const User& other);
    User& operator=(const User& other);
    User(User&& other) noexcept;
    User& operator=(User&& other) noexcept;

    std::string get_uuid() const;
    
private:
    std::string m_uuid;
};

} // namespace user


