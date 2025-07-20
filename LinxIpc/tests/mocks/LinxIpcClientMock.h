#include <gmock/gmock.h>
#include "LinxIpc.h"

class LinxIpcClientMock : public LinxIpcClient {
  public:
    LinxIpcClientMock() {
        ON_CALL(*this, operatorEqual(testing::_)).WillByDefault(testing::Invoke([this](const LinxIpcClient &to) {
            return this->getName() == to.getName();
        }));
    }

    MOCK_METHOD(int, send, (const LinxMessageIpc &message));
    MOCK_METHOD(LinxMessageIpcPtr, receive, (int timeoutMs, const std::vector<uint32_t> &sigsel));
    MOCK_METHOD(std::string, getName, (), (const));
    MOCK_METHOD(bool, connect, (int timeout));

    MOCK_METHOD(bool, operatorEqual, (const LinxIpcClient &to), (const));
    bool operator==(const LinxIpcClient &to) const override { return operatorEqual(to); }
    bool operator!=(const LinxIpcClient &to) const override { return !operatorEqual(to); }
};
