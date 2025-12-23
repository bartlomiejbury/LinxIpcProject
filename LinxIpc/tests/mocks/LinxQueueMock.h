#include <gmock/gmock.h>
#include "LinxEventFdMock.h"
#include "LinxIpc.h"
#include "LinxQueue.h"

class LinxQueueMock : public LinxQueue {
  public:
    LinxQueueMock() : LinxQueue(std::make_unique<testing::NiceMock<LinxEventFdMock>>(), 10) {}

    MOCK_METHOD(int, add, (LinxReceivedMessagePtr &&msg));
    MOCK_METHOD(int, size, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
    MOCK_METHOD(void, clear, ());
    MOCK_METHOD(LinxReceivedMessagePtr, get, (int timeoutMs, const std::vector<uint32_t> &sigsel,
                                              const LinxReceiveContextOpt &from));
};
