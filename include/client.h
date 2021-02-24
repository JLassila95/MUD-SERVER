#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <string_view>
#include <cstdio>

class Client
{
    private:
    std::string mAddress;
    int mPort;
    int mSocket;
    std::string writeBuff = {' '};
    std::string readBuff = {' '};
    public:
    Client(const char* address, const int& port, const int& sock):
        mAddress(address), mPort(port),  mSocket(sock) {}
    ~Client() {}
    int getPort() const { return mPort; }
    int getSocket() const { return mSocket; }
    const std::string_view getAddress() const { return mAddress; }
    bool pendingOutput() { return writeBuff.empty(); }
    bool pendingInput() { return readBuff.empty(); }
    void processWrite();
    void processRead();
};

#endif