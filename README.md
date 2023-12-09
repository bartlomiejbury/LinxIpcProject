## Linx Ipc Library for linux

Linx IPC based on AF_UNIX sockets:

- Message have unique Signal ID (uint32_t)
- Each enpoint have separate thread to receive messages
- Endpoint can be added to poll
- Filter messages by Signal ID or sender
