#include <gmock/gmock.h>
#include "LinxIpc.h"
#include "LinxQueue.h"

class LinxIpcQueueMock : public LinxQueue {
  public:
    MOCK_METHOD(int, add, (const LinxMessageIpcPtr &msg));
    MOCK_METHOD(int, size, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
    MOCK_METHOD(void, clear, ());
    MOCK_METHOD(void, clearPending, ());
    MOCK_METHOD(LinxMessageIpcPtr, get, (int timeoutMs, const std::vector<uint32_t> &sigsel,
                                                       const std::optional<LinxIpcClientPtr> &from));
};
