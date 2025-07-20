#pragma once

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
};

/**
 * @brief Extended interface for a LINX IPC server with start/stop control. Adding thread which receives message and store them in queue.
 * During receive message is searched in queue and removed from queue if found.
 */
class LinxIpcExtendedServer : public virtual LinxIpcServer {

  public:
    virtual ~LinxIpcExtendedServer(){};

    /**
     * @brief Starts the IPC internal thread.
     */
    virtual void start() = 0;

    /**
     * @brief Stops the IPC internal thread.
     */
    virtual void stop() = 0;
};

class LinxIpcHandler {
  public:
    virtual ~LinxIpcHandler(){};
    /**
     * @brief Registers a callback function for a specific request ID.
     * @param reqId The request ID to associate with the callback.
     * @param callback The callback function to invoke when a message with reqId is received.
     * @param data User-defined data to pass to the callback.
     */
    virtual void registerCallback(uint32_t reqId, LinxIpcCallback callback, void *data) = 0;
    /**
     * @brief Handles incoming messages and dispatches them to registered callbacks.
     * @param timeoutMs Timeout in milliseconds to wait for a message. Use INFINITE_TIMEOUT for no timeout.
     * @return Status code indicating success or failure.
     */
    virtual int handleMessage(int timeoutMs = INFINITE_TIMEOUT) = 0;
};


using LinxIpcServerPtr = std::shared_ptr<LinxIpcServer>;
using LinxIpcExtendedServerPtr = std::shared_ptr<LinxIpcExtendedServer>;
using LinxIpcHandlerPtr = std::shared_ptr<LinxIpcHandler>;

LinxIpcServerPtr createLinxIpcSimpleServer(const std::string &endpointName);
LinxIpcExtendedServerPtr createLinxIpcServer(const std::string &endpointName, int maxSize = 100);
LinxIpcHandlerPtr createLinxIpcHandler(const LinxIpcServerPtr &server);