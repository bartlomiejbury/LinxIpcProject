#include <gmock/gmock.h>
#include "LinxIpc.h"

class LinxClientMock : public LinxClient {
  public:
    LinxClientMock(const std::string &clientName) : clientName{clientName} {}
    MOCK_METHOD(int, send, (const IMessage &message), (override));
    MOCK_METHOD(RawMessagePtr, receive, (int timeoutMs, const std::vector<uint32_t> &sigsel), (override));
    MOCK_METHOD(RawMessagePtr, sendReceive, (const IMessage &message, int timeoutMs, const std::vector<uint32_t> &sigsel), (override));
    MOCK_METHOD(bool, connect, (int timeout), (override));
    MOCK_METHOD(std::string, getName, (), (const, override));

    bool isEqual(const LinxClient &other) const override {
        const LinxClientMock &otherContext = static_cast<const LinxClientMock &>(other);
        return this->clientName == otherContext.clientName;
    }

    std::string clientName;
};
