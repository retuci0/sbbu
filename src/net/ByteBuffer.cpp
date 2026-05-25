#include "ByteBuffer.h"

#include <arpa/inet.h>
#include <stdexcept>


/* --- WRITE --- */

void ByteBuffer::writeUint16(uint16_t v) {
    uint16_t net = htons(v);
    writeBytes(&net, 2);
}

void ByteBuffer::writeUint32(uint32_t v) {
    uint32_t net = htonl(v);
    writeBytes(&net, 4);
}

void ByteBuffer::writeFloat(float v) {
    uint32_t tmp;
    std::memcpy(&tmp, &v, 4);
    writeUint32(tmp);
}

void ByteBuffer::writeString(const std::string& s) {
    writeUint16(static_cast<uint16_t>(s.size()));
    writeBytes(s.data(), s.size());
}

void ByteBuffer::writeBytes(const void* data, size_t len) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    buffer.insert(buffer.end(), bytes, bytes + len);
}


/* --- READ --- */

uint8_t ByteBuffer::readUint8() {
    ensureReadable(1);
    return buffer[readPos++];
}

uint16_t ByteBuffer::readUint16() {
    ensureReadable(2);
    uint16_t v;
    std::memcpy(&v, buffer.data() + readPos, 2);
    readPos += 2;
    return ntohs(v);
}

uint32_t ByteBuffer::readUint32() {
    ensureReadable(4);
    uint32_t v;
    std::memcpy(&v, buffer.data() + readPos, 4);
    readPos += 4;
    return ntohl(v);
}

float ByteBuffer::readFloat() {
    uint32_t raw = readUint32();
    float v;
    std::memcpy(&v, &raw, 4);
    return v;
}

std::string ByteBuffer::readString() {
    uint16_t len = readUint16();
    ensureReadable(len);
    std::string s(reinterpret_cast<const char*>(buffer.data() + readPos), len);
    readPos += len;
    return s;
}

void ByteBuffer::readBytes(void* out, size_t len) {
    ensureReadable(len);
    std::memcpy(out, buffer.data() + readPos, len);
    readPos += len;
}

void ByteBuffer::ensureReadable(size_t len) {
    if (readPos + len > buffer.size()) {
        throw std::runtime_error("ByteBuffer underflow");
    }
}