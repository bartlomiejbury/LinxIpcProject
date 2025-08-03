#pragma once

#include <optional>
/**
 * @brief Abstract interface for a LINX IPC server.
 *
 * LinxIpcEndpoint defines the interface for an inter-process communication (IPC) server
 * using the LINX protocol.
 */
class LinxIpcEndpoint {

  public:
    virtual ~LinxIpcEndpoint(){};

    virtual int send(const LinxMessageIpc &message, const std::string &to) = 0;
    virtual LinxMessageIpcPtr receive(int timeoutMs, 
                                      const std::vector<uint32_t> &sigsel,
                                      const std::optional<std::string> &from) = 0;
    
    virtual int getPollFd() const = 0;
    virtual std::string getName() const = 0;
};

typedef std::shared_ptr<LinxIpcEndpoint> LinxIpcEndpointPtr;