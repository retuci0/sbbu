#pragma once

#include "net/Packets.h"

#include <memory>
#include <unordered_map>
#include <functional>


class PacketRegistry {
public:
    using Factory = std::function<std::unique_ptr<Packet>()>;

    static PacketRegistry& instance() {
        static PacketRegistry reg;
        return reg;
    }

    void registerPacket(PacketType type, Factory factory) {
        factories[type] = std::move(factory);
    }

    std::unique_ptr<Packet> createPacket(PacketType type) const {
        auto it = factories.find(type);
        if (it == factories.end()) return nullptr;
        return it->second();
    }

private:
    PacketRegistry() {
        // register all packet types
        registerPacket(PacketType::HANDSHAKE,      []{ return std::make_unique<HandshakePacket>(); });
        registerPacket(PacketType::HANDSHAKE_ACK,  []{ return std::make_unique<HandshakeAckPacket>(); });
        registerPacket(PacketType::CLIENT_INPUT,   []{ return std::make_unique<ClientInputPacket>(); });
        registerPacket(PacketType::STATE_UPDATE,   []{ return std::make_unique<StateUpdatePacket>(); });
        registerPacket(PacketType::GAME_SETUP,     []{ return std::make_unique<GameSetupPacket>(); });
        registerPacket(PacketType::DISCONNECT,     []{ return std::make_unique<DisconnectPacket>(); });
        registerPacket(PacketType::PING,           []{ return std::make_unique<PingPacket>(); });
        registerPacket(PacketType::PONG,           []{ return std::make_unique<PongPacket>(); });
    }

    std::unordered_map<PacketType, Factory> factories;
};
