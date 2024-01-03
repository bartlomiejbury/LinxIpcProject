#include "LinxIpc.h"
#include "LinxQueue.h"
#include "CMock2.h"

class LinxIpcQueueMock : public LinxQueue {
  public:
    MOCK_METHOD(int, add, (const LinxMessageIpcPtr &msg, const std::string &from));
    MOCK_METHOD(int, size, (), (const));
    MOCK_METHOD(int, getFd, (), (const));
    MOCK_METHOD(void, clear, ());
    MOCK_METHOD(void, clearPending, ());
    MOCK_METHOD(std::shared_ptr<LinxQueueContainer>, get, (int timeoutMs, const std::initializer_list<uint32_t> &sigsel,
                                                    const std::optional<std::string> &from));
};
