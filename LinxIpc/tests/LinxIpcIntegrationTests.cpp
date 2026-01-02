#include <thread>
#include "gtest/gtest.h"
#include "AfUnix.h"
#include "UdpLinx.h"

using namespace ::testing;

class LinxIpcIntegrationTests : public testing::Test {
  public:
};

static const uint32_t IPC_SIG1_REQ = IPC_SIG_BASE + 1;
static const uint32_t IPC_SIG1_RSP = IPC_SIG_BASE + 2;
static const uint32_t IPC_SIG2_REQ = IPC_SIG_BASE + 3;
static const uint32_t IPC_SIG2_RSP = IPC_SIG_BASE + 4;

TEST_F(LinxIpcIntegrationTests, testHandleMessageUnix) {

    std::atomic<bool> running{true};
    std::thread handlerThread([&]() {
        auto server = AfUnixFactory::createServer("TestService", 10);
        auto handler = LinxIpcHandler(server);
        handler.registerCallback(IPC_SIG1_REQ, [&handler](const LinxReceivedMessageSharedPtr &msg, void *data) {
            RawMessage rsp(IPC_SIG1_RSP);
            handler.send(rsp, *msg->from);
            return 0;
        }, nullptr)
        .registerCallback(IPC_SIG2_REQ, [&handler](const LinxReceivedMessageSharedPtr &msg, void *data) {
            RawMessage rsp(IPC_SIG2_RSP);
            handler.send(rsp, *msg->from);
            return 0;
        }, nullptr);

        ASSERT_TRUE(handler.start());
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    // Ensure thread is always joined, even if test fails
    auto threadGuard = [&]() {
        running = false;
        if (handlerThread.joinable()) {
            handlerThread.join();
        }
    };
    std::shared_ptr<void> guard(nullptr, [&](void*) { threadGuard(); });

    auto client = AfUnixFactory::createClient("TestService");
    ASSERT_TRUE(client->connect(1000));

    auto startTime = std::chrono::steady_clock::now();
    auto rsp = client->sendReceive(RawMessage(IPC_SIG1_REQ));
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    running = false;
    handlerThread.join();

    ASSERT_NE(rsp, nullptr);
    ASSERT_EQ(rsp->getReqId(), IPC_SIG1_RSP);
    ASSERT_LE(duration.count(), 10) << "Get should not take more than " << 10 <<" ms";
}

TEST_F(LinxIpcIntegrationTests, testHandleMessageUdp) {

    const std::string LINX_MULTICAST_IP_ADDRESS = "239.0.0.1";

    std::atomic<bool> running{true};
    std::thread handlerThread([&]() {
        auto server = UdpFactory::createMulticastServer(LINX_MULTICAST_IP_ADDRESS, 12345, 10);
        auto handler = LinxIpcHandler(server);
        handler.registerCallback(IPC_SIG1_REQ, [&handler](const LinxReceivedMessageSharedPtr &msg, void *data) {
            RawMessage rsp(IPC_SIG1_RSP, {1, 2});
            handler.send(rsp, *msg->from);
            return 0;
        }, nullptr)
        .registerCallback(IPC_SIG2_REQ, [&handler](const LinxReceivedMessageSharedPtr &msg, void *data) {
            RawMessage rsp(IPC_SIG2_RSP);
            handler.send(rsp, *msg->from);
            return 0;
        }, nullptr);

        ASSERT_TRUE(handler.start());
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    // Ensure thread is always joined, even if test fails
    auto threadGuard = [&]() {
        running = false;
        if (handlerThread.joinable()) {
            handlerThread.join();
        }
    };
    std::shared_ptr<void> guard(nullptr, [&](void*) { threadGuard(); });

    //auto client = UdpFactory::createClient("127.0.0.1", 12345);
    auto client = UdpFactory::createClient(LINX_MULTICAST_IP_ADDRESS, 12345);
    ASSERT_TRUE(client->connect(1000));

    auto startTime = std::chrono::steady_clock::now();
    auto rsp = client->sendReceive(RawMessage(IPC_SIG1_REQ, {1, 2}));
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    running = false;
    handlerThread.join();

    ASSERT_NE(rsp, nullptr);
    ASSERT_EQ(rsp->getReqId(), IPC_SIG1_RSP);
    ASSERT_LE(duration.count(), 10) << "Get should not take more than " << 10 <<" ms";
}

TEST_F(LinxIpcIntegrationTests, testConnectTwoServers) {

    std::atomic<bool> running{true};
    std::thread handlerThread([&]() {
        auto server = AfUnixFactory::createServer("TestService", 10);
        auto handler = LinxIpcHandler(server);
        handler.registerCallback(IPC_SIG1_REQ, [&handler](const LinxReceivedMessageSharedPtr &msg, void *data) {
            RawMessage rsp(IPC_SIG1_RSP);
            handler.send(rsp, *msg->from);
            return 0;
        }, nullptr)
        .registerCallback(IPC_SIG2_REQ, [&handler](const LinxReceivedMessageSharedPtr &msg, void *data) {
            RawMessage rsp(IPC_SIG2_RSP);
            handler.send(rsp, *msg->from);
            return 0;
        }, nullptr);

        ASSERT_TRUE(handler.start());
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    // Ensure thread is always joined, even if test fails
    auto threadGuard = [&]() {
        running = false;
        if (handlerThread.joinable()) {
            handlerThread.join();
        }
    };
    std::shared_ptr<void> guard(nullptr, [&](void*) { threadGuard(); });

    auto secondServer = AfUnixFactory::createServer("TestClient");
    ASSERT_TRUE(secondServer->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto startTime = std::chrono::steady_clock::now();
    secondServer->send(RawMessage(IPC_SIG1_REQ), StringIdentifier("TestService"));
    auto rsp1 = secondServer->receive(1000, {IPC_SIG1_RSP}, LINX_ANY_FROM);
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    ASSERT_NE(rsp1, nullptr);
    ASSERT_EQ(rsp1->message->getReqId(), IPC_SIG1_RSP);
    // Cannot compare identifier directly with client anymore - identifier contains the socket path
    ASSERT_LE(duration.count(), 10) << "Get should not take more than " << 10 <<" ms";

    running = false;
    handlerThread.join();

    secondServer->stop();
}

TEST_F(LinxIpcIntegrationTests, TestFailToCreateTwoServersWithSameName) {

    auto server1 = AfUnixFactory::createServer("TestService", 10);
    ASSERT_NE(server1, nullptr);

    auto server2 = AfUnixFactory::createServer("TestService", 10);
    ASSERT_EQ(server2, nullptr) << "Should not be able to create two servers with the same name";
}

TEST_F(LinxIpcIntegrationTests, TestFailToSendMessageToNonExistingServer) {

    auto client = AfUnixFactory::createClient("TestService");

    auto rsp = client->sendReceive(RawMessage(IPC_SIG1_REQ));
    ASSERT_EQ(rsp, nullptr);
}

TEST_F(LinxIpcIntegrationTests, TestFailToConnectToNonExistingServer) {

    auto client = AfUnixFactory::createClient("TestService");

    auto rsp = client->connect(100);
    ASSERT_FALSE(rsp);
}
