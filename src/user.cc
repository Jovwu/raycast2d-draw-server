#include "user.h"

namespace user{

User::User() : m_uuid(tool::GenerateUUID()) {}

User::~User() {}

User::User(const User& other) : m_uuid(other.m_uuid) {}

User& User::operator=(const User& other) {
    if (this != &other) {
        m_uuid = other.m_uuid;
    }
    return *this;
}

User::User(User&& other) noexcept : m_uuid(std::move(other.m_uuid)) {}

User& User::operator=(User&& other) noexcept {
    if (this != &other) {
        m_uuid = std::move(other.m_uuid);
    }
    return *this;
}

std::string User::get_uuid() const {
    return m_uuid;
}

}