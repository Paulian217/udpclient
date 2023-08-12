#ifndef WORKSPACE_INCLUDE_ISOCKETLISTENER_H
#define WORKSPACE_INCLUDE_ISOCKETLISTENER_H

#include <arpa/inet.h>

#include <vector>

class ISocketListener {
public:
    virtual void onBufferReceived(const struct sockaddr_in& addr, const std::vector<unsigned char>& buffer) = 0;
};

#endif  // WORKSPACE_INCLUDE_ISOCKETLISTENER_H