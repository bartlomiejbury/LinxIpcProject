#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

class IIdentifier;
class IMessage;

namespace LinxMessageFilter {

// Check if a message matches the signal selector
// Returns true if sigsel is empty (match any) or message reqId is in sigsel
inline bool matchesSignalSelector(const IMessage &msg, const std::vector<uint32_t> &sigsel) {
    uint32_t reqId = msg.getReqId();
    return sigsel.size() == 0 ||
           std::find_if(sigsel.begin(), sigsel.end(),
                       [reqId](uint32_t id) { return id == reqId; }) != sigsel.end();
}

// Check if a sender identifier matches an expected identifier
// Returns true if expected is nullptr (any sender) or if from matches expected
inline bool matchesFrom(const IIdentifier *from, const IIdentifier *expected) {
    if (expected == nullptr) {
        return true;  // Accept from any sender
    }
    if (from == nullptr) {
        return false;  // Expected specific sender but got nullptr
    }
    return *from == *expected;
}

} // namespace LinxMessageFilter
