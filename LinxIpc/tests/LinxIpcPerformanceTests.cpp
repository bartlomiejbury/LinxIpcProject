#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/utsname.h>
#include "gtest/gtest.h"
#include "AfUnix.h"

using namespace ::testing;
using namespace std::chrono;

class LinxIpcPerformanceTests : public testing::Test {
  public:
    static constexpr uint32_t PERF_SIG_REQ = IPC_SIG_BASE + 100;
    static constexpr uint32_t PERF_SIG_RSP = IPC_SIG_BASE + 101;
    static constexpr int labelWidth = 20;

    static void SetUpTestSuite() {
        printMachineInfo();
    }

    static void printMachineInfo() {
        std::cout << "\n==================== Machine Information ====================\n";

        // Get system information
        struct utsname unameData;
        if (uname(&unameData) == 0) {
            std::cout << std::left << std::setw(labelWidth) << "OS:" << unameData.sysname << " " << unameData.release << "\n";
            std::cout << std::left << std::setw(labelWidth) << "Kernel:" << unameData.version << "\n";
            std::cout << std::left << std::setw(labelWidth) << "Architecture:" << unameData.machine << "\n";
        }

        // Get CPU information
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (cpuinfo.is_open()) {
            std::string line;
            std::string cpuModel;
            int cpuCount = 0;

            while (std::getline(cpuinfo, line)) {
                if (line.find("model name") != std::string::npos) {
                    if (cpuModel.empty()) {
                        size_t pos = line.find(":");
                        if (pos != std::string::npos) {
                            cpuModel = line.substr(pos + 2);
                        }
                    }
                    cpuCount++;
                }
            }

            if (!cpuModel.empty()) {
                std::cout << std::left << std::setw(labelWidth) << "CPU:" << cpuModel << "\n";
                std::cout << std::left << std::setw(labelWidth) << "CPU Cores:" << cpuCount << "\n";
            }
            cpuinfo.close();
        }

        // Get memory information
        std::ifstream meminfo("/proc/meminfo");
        if (meminfo.is_open()) {
            std::string line;
            while (std::getline(meminfo, line)) {
                if (line.find("MemTotal:") != std::string::npos) {
                    size_t pos = line.find(":");
                    if (pos != std::string::npos) {
                        std::string memValue = line.substr(pos + 1);
                        // Trim leading spaces
                        memValue.erase(0, memValue.find_first_not_of(" \t"));
                        std::cout << std::left << std::setw(labelWidth) << "Total Memory:" << memValue << "\n";
                    }
                    break;
                }
            }
            meminfo.close();
        }

        // Check power supply status
        FILE* pipe = popen("cat /sys/class/power_supply/AC*/online 2>/dev/null", "r");
        if (pipe) {
            char buffer[128];
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            int exitCode = pclose(pipe);

            if (exitCode == 0 && !result.empty()) {
                int status = std::stoi(result);
                std::cout << std::left << std::setw(labelWidth) << "Power Supply:" << (status == 1 ? "Plugged In (AC)" : "Unplugged (Battery)") << "\n";
            } else {
                std::cout << std::left << std::setw(labelWidth) << "Power Supply:" << "Unable to detect (Desktop or unsupported system)" << "\n";
            }
        } else {
            std::cout << std::left << std::setw(labelWidth) << "Power Supply:" << "Unable to detect (Desktop or unsupported system)" << "\n";
        }

        std::cout << "=============================================================\n\n";
    }
};

TEST_F(LinxIpcPerformanceTests, Throughput_MessageRate) {
    std::atomic<bool> running{true};
    std::atomic<int> messageCount{0};

    // Start server with handler
    std::thread serverThread([&]() {
        auto server = AfUnixFactory::createServer("PerfTestServer", 1000);
        auto handler = LinxIpcHandler(server);

        handler.registerCallback(PERF_SIG_REQ,
            [&](const LinxReceivedMessageSharedPtr &msg, void *data) {
                messageCount++;
                RawMessage rsp(PERF_SIG_RSP);
                handler.send(rsp, *msg->from);
                return 0;
            }, nullptr);

        handler.start();
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    std::this_thread::sleep_for(milliseconds(100));

    auto client = AfUnixFactory::createClient("PerfTestServer");
    ASSERT_TRUE(client->connect(5000));

    // Warm up
    for (int i = 0; i < 100; i++) {
        RawMessage msg(PERF_SIG_REQ);
        client->sendReceive(msg, 1000, {PERF_SIG_RSP});
    }

    messageCount = 0;
    auto start = high_resolution_clock::now();

    // Performance test - send 10000 messages
    const int totalMessages = 10000;
    for (int i = 0; i < totalMessages; i++) {
        RawMessage msg(PERF_SIG_REQ);
        auto rsp = client->sendReceive(msg, 1000, {PERF_SIG_RSP});
        ASSERT_NE(rsp, nullptr);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    running = false;
    serverThread.join();

    double messagesPerSecond = (totalMessages * 1000.0) / duration;
    double avgLatencyMs = (double)duration / totalMessages;

    std::cout << "\n=== Performance Results ===\n";
    std::cout << std::left << std::setw(labelWidth) << "Total messages:" << totalMessages << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Total time:" << duration << " ms\n";
    std::cout << std::left << std::setw(labelWidth) << "Throughput:" << messagesPerSecond << " msg/s\n";
    std::cout << std::left << std::setw(labelWidth) << "Average latency:" << avgLatencyMs << " ms\n";
    std::cout << "===========================\n";

    EXPECT_EQ(messageCount, totalMessages);
    EXPECT_GT(messagesPerSecond, 1000) << "Throughput should be > 1000 msg/s";
}

TEST_F(LinxIpcPerformanceTests, Latency_SingleMessage) {
    std::atomic<bool> running{true};

    std::thread serverThread([&]() {
        auto server = AfUnixFactory::createServer("LatencyTestServer");
        auto handler = LinxIpcHandler(server);

        handler.registerCallback(PERF_SIG_REQ,
            [&](const LinxReceivedMessageSharedPtr &msg, void *data) {
                RawMessage rsp(PERF_SIG_RSP);
                handler.send(rsp, *msg->from);
                return 0;
            }, nullptr);

        handler.start();
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    std::this_thread::sleep_for(milliseconds(100));

    auto client = AfUnixFactory::createClient("LatencyTestServer");
    ASSERT_TRUE(client->connect(5000));

    // Warm up
    for (int i = 0; i < 10; i++) {
        RawMessage msg(PERF_SIG_REQ);
        client->sendReceive(msg, 1000, {PERF_SIG_RSP});
    }

    // Measure minimum latency over 100 samples
    auto minLatency = duration<double, std::milli>::max();
    auto maxLatency = duration<double, std::milli>::min();
    double totalLatency = 0;
    const int samples = 100;

    for (int i = 0; i < samples; i++) {
        auto start = high_resolution_clock::now();

        RawMessage msg(PERF_SIG_REQ);
        auto rsp = client->sendReceive(msg, 1000, {PERF_SIG_RSP});

        auto end = high_resolution_clock::now();
        auto latency = duration_cast<duration<double, std::milli>>(end - start);

        ASSERT_NE(rsp, nullptr);

        minLatency = std::min(minLatency, latency);
        maxLatency = std::max(maxLatency, latency);
        totalLatency += latency.count();
    }

    running = false;
    serverThread.join();

    double avgLatency = totalLatency / samples;

    std::cout << "\n=== Latency Results ===\n";
    std::cout << std::left << std::setw(labelWidth) << "Samples:" << samples << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Min latency:" << minLatency.count() << " ms\n";
    std::cout << std::left << std::setw(labelWidth) << "Max latency:" << maxLatency.count() << " ms\n";
    std::cout << std::left << std::setw(labelWidth) << "Avg latency:" << avgLatency << " ms\n";
    std::cout << "=======================\n";

    EXPECT_LT(avgLatency, 10.0) << "Average latency should be < 10 ms";
}

TEST_F(LinxIpcPerformanceTests, Throughput_LargePayload) {
    std::atomic<bool> running{true};

    std::thread serverThread([&]() {
        auto server = AfUnixFactory::createServer("LargePayloadServer", 500);
        auto handler = LinxIpcHandler(server);

        handler.registerCallback(PERF_SIG_REQ,
            [&](const LinxReceivedMessageSharedPtr &msg, void *data) {
                // Echo back the payload
                auto payload = msg->message->getPayload();
                auto size = msg->message->getPayloadSize();
                RawMessage rsp(PERF_SIG_RSP, payload, size);
                handler.send(rsp, *msg->from);
                return 0;
            }, nullptr);

        handler.start();
        while (running) {
            handler.handleMessage(100);
        }
        handler.stop();
    });

    std::this_thread::sleep_for(milliseconds(100));

    auto client = AfUnixFactory::createClient("LargePayloadServer");
    ASSERT_TRUE(client->connect(5000));

    // Test with 64KB payload
    const size_t payloadSize = 64 * 1024;
    std::vector<uint8_t> largePayload(payloadSize);
    for (size_t i = 0; i < payloadSize; i++) {
        largePayload[i] = static_cast<uint8_t>(i % 256);
    }

    // Warm up
    for (int i = 0; i < 10; i++) {
        RawMessage msg(PERF_SIG_REQ, largePayload.data(), payloadSize);
        client->sendReceive(msg, 2000, {PERF_SIG_RSP});
    }

    auto start = high_resolution_clock::now();

    const int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        RawMessage msg(PERF_SIG_REQ, largePayload.data(), payloadSize);
        auto rsp = client->sendReceive(msg, 2000, {PERF_SIG_RSP});
        ASSERT_NE(rsp, nullptr);
        ASSERT_EQ(rsp->getPayloadSize(), payloadSize);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    running = false;
    serverThread.join();

    double throughputMBps = (iterations * payloadSize * 1000.0) / (duration * 1024 * 1024);
    double avgLatencyMs = (double)duration / iterations;

    std::cout << "\n=== Large Payload Performance ===\n";
    std::cout << std::left << std::setw(labelWidth) << "Payload size:" << payloadSize << " bytes\n";
    std::cout << std::left << std::setw(labelWidth) << "Iterations:" << iterations << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Total time:" << duration << " ms\n";
    std::cout << std::left << std::setw(labelWidth) << "Throughput:" << throughputMBps << " MB/s\n";
    std::cout << std::left << std::setw(labelWidth) << "Avg latency:" << avgLatencyMs << " ms\n";
    std::cout << "==================================\n";

    EXPECT_GT(throughputMBps, 50.0) << "Throughput should be > 50 MB/s";
}

TEST_F(LinxIpcPerformanceTests, Concurrency_MultipleClients) {
    std::atomic<bool> running{true};
    std::atomic<int> totalMessages{0};

    std::thread serverThread([&]() {
        auto server = AfUnixFactory::createServer("ConcurrencyTestServer", 2000);
        auto handler = LinxIpcHandler(server);

        handler.registerCallback(PERF_SIG_REQ,
            [&](const LinxReceivedMessageSharedPtr &msg, void *data) {
                totalMessages++;
                RawMessage rsp(PERF_SIG_RSP);
                handler.send(rsp, *msg->from);
                return 0;
            }, nullptr);

        handler.start();
        while (running) {
            handler.handleMessage(10);
        }
        handler.stop();
    });

    std::this_thread::sleep_for(milliseconds(100));

    const int numClients = 10;
    const int messagesPerClient = 500;
    std::vector<std::thread> clientThreads;

    auto start = high_resolution_clock::now();

    for (int c = 0; c < numClients; c++) {
        clientThreads.emplace_back([c, messagesPerClient]() {
            auto client = AfUnixFactory::createClient("ConcurrencyTestServer");
            ASSERT_TRUE(client->connect(5000));

            for (int i = 0; i < messagesPerClient; i++) {
                RawMessage msg(PERF_SIG_REQ);
                auto rsp = client->sendReceive(msg, 2000, {PERF_SIG_RSP});
                ASSERT_NE(rsp, nullptr);
            }
        });
    }

    for (auto& thread : clientThreads) {
        thread.join();
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    running = false;
    serverThread.join();

    int expectedMessages = numClients * messagesPerClient;
    double messagesPerSecond = (expectedMessages * 1000.0) / duration;

    std::cout << "\n=== Concurrency Performance ===\n";
    std::cout << std::left << std::setw(labelWidth) << "Clients:" << numClients << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Messages per client:" << messagesPerClient << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Total messages:" << expectedMessages << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Total time:" << duration << " ms\n";
    std::cout << std::left << std::setw(labelWidth) << "Throughput:" << messagesPerSecond << " msg/s\n";
    std::cout << "===============================\n";

    EXPECT_EQ(totalMessages, expectedMessages);
    EXPECT_GT(messagesPerSecond, 500) << "Concurrent throughput should be > 500 msg/s";
}
