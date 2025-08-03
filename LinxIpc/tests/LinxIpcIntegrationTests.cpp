
#include <stdio.h>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "LinxIpc.h"
#include "LinxQueueImpl.h"
#include "LinxIpcEventFdImpl.h"

using namespace ::testing;

class LinxIpcIntegrationTests : public testing::Test {
  public:
};

static const uint32_t IPC_SIG1_REQ = IPC_SIG_BASE + 1;
static const uint32_t IPC_SIG1_RSP = IPC_SIG_BASE + 2;
static const uint32_t IPC_SIG2_REQ = IPC_SIG_BASE + 3;
static const uint32_t IPC_SIG2_RSP = IPC_SIG_BASE + 4;

TEST_F(LinxIpcIntegrationTests, testHandleMessage) {

    bool running = true;
    std::thread handlerThread([&]() {
        auto handler = LinxIpcHandlerBuilder("TestService")
        .registerCallback(IPC_SIG1_REQ, [](LinxMessageIpc *msg, void *data) {
            LinxMessageIpc rsp(IPC_SIG1_RSP);
            msg->getClient()->send(rsp);
            
            return 0;
        }, nullptr)
        .registerCallback(IPC_SIG2_REQ, [](LinxMessageIpc *msg, void *data) {
            LinxMessageIpc rsp(IPC_SIG2_RSP);
            msg->getClient()->send(rsp);
            return 0;
        }, nullptr)
        .build();

        while (running) {
            handler->handleMessage(100);
        }
        handler->stop();
    });

    auto client = createIpcClient("TestService");
    ASSERT_TRUE(client->connect(1000));

    auto startTime = std::chrono::steady_clock::now();
    auto rsp = client->sendReceive(LinxMessageIpc(IPC_SIG1_REQ));
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    running = false;
    handlerThread.join();

    ASSERT_NE(rsp, nullptr);
    ASSERT_EQ(rsp->getReqId(), IPC_SIG1_RSP);
    ASSERT_LE(duration.count(), 10) << "Get should not take more than " << 10 <<" ms";
}

TEST_F(LinxIpcIntegrationTests, TestFailToCreateTwoServewWithSameName) {

    auto server1 = createIpcServer("TestService", 10);
    ASSERT_NE(server1, nullptr);

    auto server2 = createIpcServer("TestService", 10);
    ASSERT_EQ(server2, nullptr) << "Should not be able to create two servers with the same name";
}

TEST_F(LinxIpcIntegrationTests, TestFailToSendMessageToNonExistingServer) {

    auto client = createIpcClient("TestService");

    auto rsp = client->sendReceive(LinxMessageIpc(IPC_SIG1_REQ));
    ASSERT_EQ(rsp, nullptr);
}

