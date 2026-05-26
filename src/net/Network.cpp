#include "Network.h"

#include "PacketRegistry.h"

#include <SDL2/SDL.h>

#include <cstring>


static constexpr int MAX_PACKET_SIZE  = 2048;
static constexpr int PROTOCOL_VERSION =    1;

std::unique_ptr<Network> Network::createHost(uint16_t port) {
    auto net = std::unique_ptr<Network>(new Network());
    if (net->initAsHost(port)) return net;
    return nullptr;
}

std::unique_ptr<Network> Network::createClient(const char* hostIp, uint16_t port) {
    auto net = std::unique_ptr<Network>(new Network());
    if (net->initAsClient(hostIp, port)) return net;
    return nullptr;
}

Network::~Network() {
    disconnect();
    if (socket) SDLNet_UDP_Close(socket);
}

bool Network::initAsHost(uint16_t port) {
    socket = SDLNet_UDP_Open(port);
    if (!socket) return false;
    lastReceiveTicks = SDL_GetTicks();
    return true;
}

bool Network::initAsClient(const char* hostIp, uint16_t port) {
    socket = SDLNet_UDP_Open(0);
    if (!socket) return false;
    if (SDLNet_ResolveHost(&remoteAddr, hostIp, port) < 0) return false;
    lastReceiveTicks = SDL_GetTicks();
    sendHandshake();
    return true;
}

void Network::disconnect(bool notifyPeer) {
    if (connected && notifyPeer) {
        DisconnectPacket pkt;
        send(pkt);
    }
    if (connected && onDisconnect) onDisconnect();
    connected = false;
}

void Network::sendHandshake() {
    HandshakePacket pkt(PROTOCOL_VERSION);
    send(pkt);
}

void Network::poll() {
    if (!socket) return;
    UDPpacket* up = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!up) return;

    while (SDLNet_UDP_Recv(socket, up)) {
        if (up->len < 1) continue;
        lastReceiveTicks = SDL_GetTicks();

        PacketType type = static_cast<PacketType>(up->data[0]);
        auto pkt = PacketRegistry::instance().createPacket(type);
        if (!pkt) continue;

        // deserialise payload
        std::vector<uint8_t> payload(up->data + 1, up->data + up->len);
        ByteBuffer buf(payload);
        try {
            pkt->read(buf);
        } catch (...) {
            continue;  // malformed packet
        }

        // handle handshake automatically
        if (!connected && (type == PacketType::HANDSHAKE || type == PacketType::HANDSHAKE_ACK)) {
            remoteAddr = up->address;
            connected = true;
            if (onConnect) onConnect();
            if (type == PacketType::HANDSHAKE) {
                HandshakeAckPacket ack;
                send(ack);
            }
        }

        pushPacket(std::move(pkt));
    }
    SDLNet_FreePacket(up);
}

bool Network::recv(std::unique_ptr<Packet>& out) {
    if (recvQueue.empty()) return false;
    out = std::move(recvQueue.front());
    recvQueue.pop();
    return true;
}

void Network::send(const Packet& pkt) {
    if (!socket) return;
    if (!connected && pkt.getType() != PacketType::HANDSHAKE) return;
    auto data = pkt.encode();
    if (data.empty() || data.size() > MAX_PACKET_SIZE) return;

    UDPpacket* up = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!up) return;
    up->address = remoteAddr;
    up->len = static_cast<int>(data.size());
    std::memcpy(up->data, data.data(), data.size());
    SDLNet_UDP_Send(socket, -1, up);
    SDLNet_FreePacket(up);
}

void Network::pushPacket(std::unique_ptr<Packet> pkt) {
    recvQueue.push(std::move(pkt));
}