#pragma once

#include <map>

/**
 * @brief Abstract interface for a LINX IPC server.
 *
 * LinxIpcServer defines the interface for an inter-process communication (IPC) server
 * using the LINX protocol.
 */
class LinxIpcServer {

  public:
    virtual ~LinxIpcServer(){};

    /**
     * @brief Sends a message to a specified IPC client.
     * @param message The message to send.
     * @param to The target client to send the message to.
     * @return Status code indicating success or failure.
     */
    virtual int send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) = 0;

    /**
     * @brief Receives a message from a client, optionally filtering by signal selectors.
     * If message received from socket not match any of the signal selectors, it will be discarded 
     * and nullptr will be returned.
     * @param timeoutMs Timeout in milliseconds to wait for a message.
     * @param sigsel List of signal selectors to filter incoming messages.
     * @param from Optional: specific client to receive from. Defaults to LINX_ANY_FROM.
     * @return Pointer to the received message, or nullptr if none received.
     */
    virtual LinxMessageIpcPtr receive(int timeoutMs = INFINITE_TIMEOUT, 
                                      const std::vector<uint32_t> &sigsel = LINX_ANY_SIG,
                                      const LinxIpcClientPtr &from = LINX_ANY_FROM) = 0;
    
    /**
     * @brief Creates a new IPC client for the specified service name.
     * @param serviceName The name of the service to connect to.
     * @return Pointer to the created IPC client.
     */
    virtual LinxIpcClientPtr createClient(const std::string &serviceName) = 0;

    /**
     * @brief Gets the file descriptor used for polling IPC events.
     * @return The poll file descriptor.
     */
    virtual int getPollFd() const = 0;

    /**
     * @brief Starts the IPC server.
     * This method should be called to initialize and start the server's operations.
     */
    virtual void start() = 0;

    /**
     * @brief Stops the IPC server.
     * This method should be called to gracefully stop the server's operations.
     */
    virtual void stop() = 0;
};

class LinxIpcHandler {
  public:
    virtual ~LinxIpcHandler(){};
    virtual int handleMessage(int timeoutMs = INFINITE_TIMEOUT) = 0;
    virtual LinxIpcClientPtr createClient(const std::string &serviceName) = 0;
};

struct IpcContainer {
    LinxIpcCallback callback;
    void *data;
};

typedef std::shared_ptr<LinxIpcHandler> LinxIpcHandlerPtr;
typedef std::shared_ptr<LinxIpcServer> LinxIpcServerPtr;

class LinxIpcHandlerBuilder {
  public:
    static LinxIpcHandlerBuilder Simple(const std::string &serverName);
    static LinxIpcHandlerBuilder Extended(const std::string &serverName, int maxSize = 100);

    LinxIpcHandlerBuilder& registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data);
    LinxIpcHandlerPtr build();

  private:
    enum class HandlerType {
        Simple,
        Extended
    };

    HandlerType type;
    std::string serverName;
    int maxSize;
    std::map<uint32_t, IpcContainer> handlers;
};