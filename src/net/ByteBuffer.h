#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>


class ByteBuffer {
public:
    ByteBuffer() = default;
    explicit ByteBuffer(const std::vector<uint8_t>& data) : buffer(data), readPos(0) {}

    // w
    void writeUint8(uint8_t v) { writeBytes(&v, 1); }
    void writeInt16(int16_t v) { writeInt16(static_cast<uint16_t>(v)); }
    void writeUint16(uint16_t v);
    void writeInt32(int32_t v) { writeUint32(static_cast<uint32_t>(v)); }
    void writeUint32(uint32_t v);
    void writeFloat(float v);
    void writeBool(bool v) { writeUint8(v ? 1 : 0); }
    void writeString(const std::string& s);
    void writeBytes(const void* data, size_t len);

    // r
    uint8_t readUint8();
    int16_t readInt16() { return static_cast<int16_t>(readUint16()); }
    uint16_t readUint16();
    int32_t readInt32() { return static_cast<int32_t>(readUint32()); }
    uint32_t readUint32();
    float readFloat();
    bool readBool() { return readUint8() != 0; }
    std::string readString();
    void readBytes(void* out, size_t len);

    const std::vector<uint8_t>& getData() const { return buffer; }
    size_t size() const { return buffer.size(); }
    void clear() { buffer.clear(); readPos = 0; }

private:
    std::vector<uint8_t> buffer;
    size_t readPos = 0;

    void ensureReadable(size_t len);
};