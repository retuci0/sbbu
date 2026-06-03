#pragma once
#include <cstdint>
#include <string>

class DiscordManager {
public:
    void init(uint64_t appId);
    void update();
    void cleanup();
    void setPresence(const std::string& details, const std::string& state);
    void clearPresence();
};