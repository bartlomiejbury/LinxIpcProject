#include "ServiceDiscovery.h"
#include "trace.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdio.h>

int main() {
    printf("Starting ServiceDiscovery Test App\n");
    auto serviceDiscovery = ServiceDiscovery::create();

    // Test 1: Register a service
    printf("Test 1: Registering service 'my-test-service' on port 8080\n");
    bool registerResult = serviceDiscovery->registerService("my-test-service", 8080);
    if (registerResult) {
        printf("[PASS] Service registered successfully\n");
    } else {
        printf("[FAIL] Failed to register service\n");
        return -1;
    }

    // Small delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 2: Discover the service
    printf("Test 2: Discovering service 'my-test-service'\n");
    auto discoveredService = serviceDiscovery->discover("my-test-service");
    if (discoveredService.has_value() && discoveredService.value().port == 8080) {
        printf("[PASS] Service discovered: %s:%d\n", discoveredService->ip.c_str(), discoveredService->port);
    } else {
        printf("[FAIL] Failed to discover service or incorrect port\n");
        return -1;
    }

    // Test 3: Discover non-existent service
    printf("Test 3: Discovering non-existent service\n");
    auto nonExistent = serviceDiscovery->discover("non-existent-service");
    if (!nonExistent.has_value()) {
        printf("[PASS] Correctly returned empty result for non-existent service\n");
    } else {
        printf("[FAIL] Unexpected result for non-existent service\n");
    }

    // Test 4: Unregister the service
    printf("Test 4: Unregistering service 'my-test-service'\n");
    bool unregisterResult = serviceDiscovery->unregisterService("my-test-service");
    if (unregisterResult) {
        printf("[PASS] Service unregistered successfully\n");
    } else {
        printf("[FAIL] Failed to unregister service\n");
        return -1;
    }

    // Test 5: Try to discover after unregistering
    printf("Test 5: Discovering service after unregistration\n");
    auto afterUnregister = serviceDiscovery->discover("my-test-service");
    if (!afterUnregister.has_value()) {
        printf("[PASS] Service correctly not found after unregistration\n");
    } else {
        printf("[FAIL] Service still found after unregistration\n");
        return -1;
    }

    printf("All tests passed!\n");
    return 0;
}
