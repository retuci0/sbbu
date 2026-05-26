#pragma once

#include "Packets.h"

#include <SDL2/SDL_net.h>

#include <functional>
#include <memory>
#include <queue>


enum class NetworkMode { 
    NONE, 
    LOCAL, 
    REMOTE_HOST, 
    REMOTE_CLIENT 
};

class Network {
public:
    using OnConnectCallback     = std::function<void()>;
    using OnDisconnectCallback  = std::function<void()>;

    static std::unique_ptr<Network> createHost(uint16_t port);
    static std::unique_ptr<Network> createClient(const char* hostIp, uint16_t port);

    ~Network();

    bool isConnected() const { return connected; }
    void disconnect();

    void poll();  // receive and process UDP
    bool recv(std::unique_ptr<Packet>& out);  // receive (pop) one packet
    void send(const Packet& pkt);  // send (push) one packet

    void setOnConnect(OnConnectCallback cb)         { onConnect    = std::move(cb); }
    void setOnDisconnect(OnDisconnectCallback cb)   { onDisconnect = std::move(cb); }

private:
    Network() = default;
    bool initAsHost(uint16_t port);
    bool initAsClient(const char* hostIp, uint16_t port);

    UDPsocket   socket = nullptr;
    IPaddress   remoteAddr;
    bool        connected = false;
    OnConnectCallback   onConnect;
    OnDisconnectCallback onDisconnect;

    std::queue<std::unique_ptr<Packet>> recvQueue;
    void pushPacket(std::unique_ptr<Packet> pkt);
    void sendHandshake();
};