#pragma once

#include "ByteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>


enum class PacketDirection : uint8_t {
    CLIENTBOUND,   // s2c
    SERVERBOUND    // c2s
};

enum class PacketType : uint8_t {
    HANDSHAKE,
    HANDSHAKE_ACK,
    CLIENT_INPUT,
    STATE_UPDATE,
    GAME_SETUP,
    DISCONNECT,
    PING,
    PONG
};

class Packet {
public:
    virtual ~Packet() = default;
    virtual PacketType getType() const = 0;
    virtual PacketDirection getDirection() const = 0;

    virtual void write(ByteBuffer& out) const = 0;
    virtual void read(ByteBuffer& in) = 0;

    std::vector<uint8_t> encode() const;
};

inline std::vector<uint8_t> Packet::encode() const {
    ByteBuffer buf;
    buf.writeUint8(static_cast<uint8_t>(getType()));
    write(buf);
    return buf.getData();
}


class HandshakePacket : public Packet {
public:
    HandshakePacket() = default;
    explicit HandshakePacket(uint32_t protocolVersion) : protocolVersion(protocolVersion) {}

    PacketType getType() const override { return PacketType::HANDSHAKE; }
    PacketDirection getDirection() const override { return PacketDirection::SERVERBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(protocolVersion);
    }
    void read(ByteBuffer& in) override {
        protocolVersion = in.readUint32();
    }

    uint32_t protocolVersion = 0;
};

class HandshakeAckPacket : public Packet {
public:
    PacketType getType() const override { return PacketType::HANDSHAKE_ACK; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {}
    void read(ByteBuffer& in) override {}
};


class ClientInputPacket : public Packet {
public:
    ClientInputPacket() = default;
    ClientInputPacket(uint32_t frame, uint8_t inputs, uint8_t lastInputs)
        : frame(frame), inputs(inputs), lastInputs(lastInputs) {}

    PacketType getType() const override { return PacketType::CLIENT_INPUT; }
    PacketDirection getDirection() const override { return PacketDirection::SERVERBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(frame);
        out.writeUint8(inputs);
        out.writeUint8(lastInputs);
    }
    void read(ByteBuffer& in) override {
        frame = in.readUint32();
        inputs = in.readUint8();
        lastInputs = in.readUint8();
    }

    uint32_t frame = 0;
    uint8_t  inputs = 0;
    uint8_t  lastInputs = 0;
};


struct PlayerState {
    float x = 0.0f, y = 0.0f, dx = 0.0f, dy = 0.0f;
    int16_t hp = 0;
    uint8_t lives = 0;      // 255 = -1 (dead)
    uint8_t status = 0;     // Status enum
    uint8_t facing = 0;     // Facing enum
    float charge = 0.0f;
    uint8_t invulnerable = 0;
    uint8_t onGround = 0;
};

struct ProjectileState {
    float x = 0.0f, y = 0.0f, velocity = 0.0f;
    uint8_t facing = 0;
    uint8_t ownerId = 0;
    uint8_t parryFreezeTimer = 0;
    uint8_t parryFlashTimer = 0;
};

class StateUpdatePacket : public Packet {
public:
    StateUpdatePacket() = default;
    StateUpdatePacket(uint32_t frame, const PlayerState& p1, const PlayerState& p2)
        : frame(frame), p1(p1), p2(p2) {}

    PacketType getType() const override { return PacketType::STATE_UPDATE; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(frame);
        auto writePlayer = [&](const PlayerState& ps) {
            out.writeFloat(ps.x); out.writeFloat(ps.y);
            out.writeFloat(ps.dx); out.writeFloat(ps.dy);
            out.writeInt16(ps.hp);
            out.writeUint8(ps.lives);
            out.writeUint8(ps.status);
            out.writeUint8(ps.facing);
            out.writeFloat(ps.charge);
            out.writeUint8(ps.invulnerable);
            out.writeUint8(ps.onGround);
        };
        writePlayer(p1);
        writePlayer(p2);

        uint8_t count = static_cast<uint8_t>(std::min<size_t>(projectiles.size(), 255));
        out.writeUint8(count);
        for (uint8_t i = 0; i < count; ++i) {
            const auto& pr = projectiles[i];
            out.writeFloat(pr.x);
            out.writeFloat(pr.y);
            out.writeFloat(pr.velocity);
            out.writeUint8(pr.facing);
            out.writeUint8(pr.ownerId);
            out.writeUint8(pr.parryFreezeTimer);
            out.writeUint8(pr.parryFlashTimer);
        }
    }
    void read(ByteBuffer& in) override {
        frame = in.readUint32();
        auto readPlayer = [&](PlayerState& ps) {
            ps.x = in.readFloat(); ps.y = in.readFloat();
            ps.dx = in.readFloat(); ps.dy = in.readFloat();
            ps.hp = in.readInt16();
            ps.lives = in.readUint8();
            ps.status = in.readUint8();
            ps.facing = in.readUint8();
            ps.charge = in.readFloat();
            ps.invulnerable = in.readUint8();
            ps.onGround = in.readUint8();
        };
        readPlayer(p1);
        readPlayer(p2);

        uint8_t count = in.readUint8();
        projectiles.clear();
        projectiles.reserve(count);
        for (uint8_t i = 0; i < count; ++i) {
            ProjectileState pr;
            pr.x = in.readFloat();
            pr.y = in.readFloat();
            pr.velocity = in.readFloat();
            pr.facing = in.readUint8();
            pr.ownerId = in.readUint8();
            pr.parryFreezeTimer = in.readUint8();
            pr.parryFlashTimer = in.readUint8();
            projectiles.push_back(pr);
        }
    }

    uint32_t frame = 0;
    PlayerState p1, p2;
    std::vector<ProjectileState> projectiles;
};


class GameSetupPacket : public Packet {
public:
    GameSetupPacket() = default;
    GameSetupPacket(uint8_t char1Idx, uint8_t char2Idx,
                    const std::string& name1, const std::string& name2,
                    uint8_t r1, uint8_t g1, uint8_t b1,
                    uint8_t r2, uint8_t g2, uint8_t b2)
        : char1Idx(char1Idx), char2Idx(char2Idx),
          name1(name1), name2(name2),
          r1(r1), g1(g1), b1(b1),
          r2(r2), g2(g2), b2(b2) {}

    PacketType getType() const override { return PacketType::GAME_SETUP; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint8(char1Idx);
        out.writeUint8(char2Idx);
        out.writeString(name1);
        out.writeString(name2);
        out.writeUint8(r1); out.writeUint8(g1); out.writeUint8(b1);
        out.writeUint8(r2); out.writeUint8(g2); out.writeUint8(b2);
    }
    void read(ByteBuffer& in) override {
        char1Idx = in.readUint8();
        char2Idx = in.readUint8();
        name1 = in.readString();
        name2 = in.readString();
        r1 = in.readUint8(); g1 = in.readUint8(); b1 = in.readUint8();
        r2 = in.readUint8(); g2 = in.readUint8(); b2 = in.readUint8();
    }

    uint8_t char1Idx = 0, char2Idx = 0;
    std::string name1, name2;
    uint8_t r1 = 0, g1 = 0, b1 = 0;
    uint8_t r2 = 0, g2 = 0, b2 = 0;
};


class DisconnectPacket : public Packet {
public:
    PacketType getType() const override { return PacketType::DISCONNECT; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {}
    void read(ByteBuffer& in) override {}
};

class PingPacket : public Packet {
public:
    PingPacket() = default;
    PingPacket(uint32_t sequence, uint32_t sentTicks)
        : sequence(sequence), sentTicks(sentTicks) {}

    PacketType getType() const override { return PacketType::PING; }
    PacketDirection getDirection() const override { return PacketDirection::SERVERBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(sequence);
        out.writeUint32(sentTicks);
    }
    void read(ByteBuffer& in) override {
        sequence = in.readUint32();
        sentTicks = in.readUint32();
    }

    uint32_t sequence = 0;
    uint32_t sentTicks = 0;
};

class PongPacket : public Packet {
public:
    PongPacket() = default;
    PongPacket(uint32_t sequence, uint32_t sentTicks)
        : sequence(sequence), sentTicks(sentTicks) {}

    PacketType getType() const override { return PacketType::PONG; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(sequence);
        out.writeUint32(sentTicks);
    }
    void read(ByteBuffer& in) override {
        sequence = in.readUint32();
        sentTicks = in.readUint32();
    }

    uint32_t sequence = 0;
    uint32_t sentTicks = 0;
};
