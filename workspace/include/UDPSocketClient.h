#ifndef DAY_1_WORKSPACE_INCLUDE_UDPSOCKETCLIENT_H
#define DAY_1_WORKSPACE_INCLUDE_UDPSOCKETCLIENT_H

#include <ISocketListener.h>
#include <SocketTypes.h>

#include <atomic>
#include <functional>
#include <thread>

class UDPSocketClient {
public:
    explicit UDPSocketClient(SocketFD& socketfd);
    virtual ~UDPSocketClient();
    SocketReturn Connect(const SocketAddress& address, const std::shared_ptr<ISocketListener>& listener = nullptr);
    SocketSize Send(const ByteBuffer& buffer);
    SocketReturn Disconnect();

private:
    SocketReturn ReceiveAndForward(const SocketFD& socketfd);

private:
    SocketFD mSocketFD;
    std::atomic<SocketStatus> mStatus;
    SocketAddress mSocketAddress;
    std::shared_ptr<ISocketListener> mSocketListener;
    std::thread mThread;
};

#endif  // DAY_1_WORKSPACE_INCLUDE_UDPSOCKETCLIENT_H