#pragma once

#include <vector>

class LinxClient {
  public:
    virtual ~LinxClient() = default;

        virtual bool operator==(const LinxClient &other) const {
        if (this == &other) {
            return true;
        }

        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return this->isEqual(other);
    };

    virtual bool operator!=(const LinxClient &other) const {
        if (this == &other) {
            return false;
        }

        if (typeid(*this) != typeid(other)) {
            return true;
        }

        return !this->isEqual(other);
    }

    virtual int send(const LinxMessage &message) = 0;
    virtual LinxMessagePtr receive(int timeoutMs, const std::vector<uint32_t> &sigsel) = 0;
    virtual LinxMessagePtr sendReceive(const LinxMessage &message, int timeoutMs = INFINITE_TIMEOUT, const std::vector<uint32_t> &sigsel = LINX_ANY_SIG) = 0;
    virtual bool connect(int timeout) = 0;
    virtual bool isEqual(const LinxClient &other) const = 0;
    virtual std::string getName() const = 0;
};
