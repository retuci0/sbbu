#pragma once

#include "net/ByteBuffer.h"
#include "misc/Common.h"

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

    uint32_t protocolVersion = PROTOCOL_VERSION;
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
    ClientInputPacket(uint32_t frame, uint16_t inputs, uint16_t lastInputs)
        : frame(frame), inputs(inputs), lastInputs(lastInputs) {}

    PacketType getType() const override { return PacketType::CLIENT_INPUT; }
    PacketDirection getDirection() const override { return PacketDirection::SERVERBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(frame);
        out.writeUint16(inputs);
        out.writeUint16(lastInputs);
    }
    void read(ByteBuffer& in) override {
        frame = in.readUint32();
        inputs = in.readUint16();
        lastInputs = in.readUint16();
    }

    uint32_t frame = 0;
    uint16_t  inputs = 0;
    uint16_t  lastInputs = 0;
};


struct PlayerNetState {
    float x = 0.0f, y = 0.0f, dx = 0.0f, dy = 0.0f;
    int16_t hp = 0;
    uint8_t lives = 0;      // 255 = -1 (dead)
    uint8_t status = 0;     // Status enum
    uint8_t facing = 0;     // Facing enum
    float charge = 0.0f;
    uint8_t invulnerable = 0;
    uint8_t onGround = 0;
};

// 0xFF = no grapple active
// targetKind: 0=none/platform, 1=player, 2=projectile, 3=point(idx in targetIndex), 4=item
struct GrappleNetState {
    uint8_t  active      = 0;     // 1 if grapple exists
    float    x           = 0.0f;
    float    y           = 0.0f;
    float    dx          = 0.0f;
    float    dy          = 0.0f;
    uint8_t  state       = 0;     // GrappleState enum
    uint8_t  targetKind  = 0;
    uint8_t  targetIndex = 0xFF;  // player id (1/2), projectile index, grapple-point index
    float    playerDx0   = 0.0f;
    float    playerDy0   = 0.0f;
    uint8_t  velocitySnapshotted = 0;
};

struct ItemNetState {
    float    x           = 0.0f;
    float    y           = 0.0f;
    uint8_t  typeIdx     = 0;  // item type index
    uint8_t  alive       = 1;
    uint8_t  active      = 1;
    float    effectTimer = 0.0f;
    float    hp          = 0.0f;
};

struct ProjectileNetState {
    float x = 0.0f, y = 0.0f, velocity = 0.0f;
    uint8_t facing = 0;
    uint8_t ownerId = 0;
    uint8_t parryFreezeTimer = 0;
    uint8_t parryFlashTimer = 0;
};

class StateUpdatePacket : public Packet {
public:
    StateUpdatePacket() = default;
    StateUpdatePacket(uint32_t frame, const PlayerNetState& p1, const PlayerNetState& p2)
        : frame(frame), p1(p1), p2(p2) {}

    PacketType getType() const override { return PacketType::STATE_UPDATE; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint32(frame);
        auto writePlayer = [&](const PlayerNetState& ps) {
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

        // grapple
        auto writeGrapple = [&](const GrappleNetState& g) {
            out.writeUint8(g.active);
            if (!g.active) return;
            out.writeFloat(g.x);
            out.writeFloat(g.y);
            out.writeFloat(g.dx);
            out.writeFloat(g.dy);
            out.writeUint8(g.state);
            out.writeUint8(g.targetKind);
            out.writeUint8(g.targetIndex);
            out.writeFloat(g.playerDx0);
            out.writeFloat(g.playerDy0);
            out.writeUint8(g.velocitySnapshotted);
        };
        writeGrapple(grapple1);
        writeGrapple(grapple2);

        // projectiles
        uint8_t projCount = static_cast<uint8_t>(std::min<size_t>(projectiles.size(), 255));
        out.writeUint8(projCount);
        for (uint8_t i = 0; i < projCount; ++i) {
            const auto& pr = projectiles[i];
            out.writeFloat(pr.x);
            out.writeFloat(pr.y);
            out.writeFloat(pr.velocity);
            out.writeUint8(pr.facing);
            out.writeUint8(pr.ownerId);
            out.writeUint8(pr.parryFreezeTimer);
            out.writeUint8(pr.parryFlashTimer);
        }

        // items
        uint8_t itemCount = static_cast<uint8_t>(std::min<size_t>(items.size(), 255));
        out.writeUint8(itemCount);
        for (uint8_t i = 0; i < itemCount; ++i) {
            const auto& it = items[i];
            out.writeFloat(it.x);
            out.writeFloat(it.y);
            out.writeUint8(it.typeIdx);
            out.writeUint8(it.alive);
            out.writeUint8(it.active);
            out.writeFloat(it.effectTimer);
            out.writeFloat(it.hp);
        }

        // platforms
        uint8_t platCount = static_cast<uint8_t>(std::min<size_t>(platformActive.size(), 255));
        out.writeUint8(platCount);
        for (uint8_t i = 0; i < platCount; ++i) {
            out.writeUint8(platformActive[i]);
        }

        // countdown
        out.writeFloat(countdownTimer);
        out.writeUint8(countdownActive);
    }

    void read(ByteBuffer& in) override {
        frame = in.readUint32();

        // players
        auto readPlayer = [&](PlayerNetState& ps) {
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

        // grapple
        auto readGrapple = [&](GrappleNetState& g) {
            g.active = in.readUint8();
            if (!g.active) return;
            g.x                  = in.readFloat();
            g.y                  = in.readFloat();
            g.dx                 = in.readFloat();
            g.dy                 = in.readFloat();
            g.state              = in.readUint8();
            g.targetKind         = in.readUint8();
            g.targetIndex        = in.readUint8();
            g.playerDx0          = in.readFloat();
            g.playerDy0          = in.readFloat();
            g.velocitySnapshotted = in.readUint8();
        };
        readGrapple(grapple1);
        readGrapple(grapple2);

        // projectiles
        uint8_t projCount = in.readUint8();
        projectiles.clear();
        projectiles.reserve(projCount);
        for (uint8_t i = 0; i < projCount; ++i) {
            ProjectileNetState pr;
            pr.x = in.readFloat();
            pr.y = in.readFloat();
            pr.velocity = in.readFloat();
            pr.facing = in.readUint8();
            pr.ownerId = in.readUint8();
            pr.parryFreezeTimer = in.readUint8();
            pr.parryFlashTimer = in.readUint8();
            projectiles.push_back(pr);
        }

        // items
        uint8_t itemCount = in.readUint8();
        items.clear();
        items.reserve(itemCount);
        for (uint8_t i = 0; i < itemCount; ++i) {
            ItemNetState it;
            it.x          = in.readFloat();
            it.y          = in.readFloat();
            it.typeIdx    = in.readUint8();
            it.alive      = in.readUint8();
            it.active     = in.readUint8();
            it.effectTimer= in.readFloat();
            it.hp         = in.readFloat();
            items.push_back(it);
        }

        // platforms
        uint8_t platCount = in.readUint8();
        platformActive.clear();
        platformActive.reserve(platCount);
        for (uint8_t i = 0; i < platCount; ++i) {
            platformActive.push_back(in.readUint8());
        }

        // countdown
        countdownTimer  = in.readFloat();
        countdownActive = in.readUint8();
    }

    uint32_t frame = 0;
    PlayerNetState p1, p2;
    GrappleNetState grapple1, grapple2;
    std::vector<ProjectileNetState> projectiles;
    std::vector<ItemNetState> items;
    std::vector<uint8_t> platformActive;  // one bit per platform: 1=active, 0=inactive
    float countdownTimer  = 0.0f;
    uint8_t countdownActive = 0;
};


class GameSetupPacket : public Packet {
public:
    GameSetupPacket() = default;
    GameSetupPacket(uint8_t char1Idx, uint8_t char2Idx,
                    const std::string& name1, const std::string& name2,
                    uint8_t r1, uint8_t g1, uint8_t b1,
                    uint8_t r2, uint8_t g2, uint8_t b2,
                    uint8_t stageIdx, bool itemsEnabled)
        : char1Idx(char1Idx), char2Idx(char2Idx),
          name1(name1), name2(name2),
          r1(r1), g1(g1), b1(b1),
          r2(r2), g2(g2), b2(b2),
          stageIdx(stageIdx), itemsEnabled(itemsEnabled) {}

    PacketType getType() const override { return PacketType::GAME_SETUP; }
    PacketDirection getDirection() const override { return PacketDirection::CLIENTBOUND; }

    void write(ByteBuffer& out) const override {
        out.writeUint8(char1Idx);
        out.writeUint8(char2Idx);
        out.writeString(name1);
        out.writeString(name2);
        out.writeUint8(r1); out.writeUint8(g1); out.writeUint8(b1);
        out.writeUint8(r2); out.writeUint8(g2); out.writeUint8(b2);
        out.writeUint8(stageIdx);
        out.writeBool(itemsEnabled);
    }
    void read(ByteBuffer& in) override {
        char1Idx = in.readUint8();
        char2Idx = in.readUint8();
        name1 = in.readString();
        name2 = in.readString();
        r1 = in.readUint8(); g1 = in.readUint8(); b1 = in.readUint8();
        r2 = in.readUint8(); g2 = in.readUint8(); b2 = in.readUint8();
        stageIdx = in.readUint8();
        itemsEnabled = in.readBool();
    }

    uint8_t char1Idx = 0, char2Idx = 0;
    uint8_t stageIdx = 0;
    std::string name1, name2;
    uint8_t r1 = 0, g1 = 0, b1 = 0;
    uint8_t r2 = 0, g2 = 0, b2 = 0;
    bool itemsEnabled = true;
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