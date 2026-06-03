#include "misc/Discord.h"

#include "discord_rpc.h"

#include <cstdio>


void DiscordManager::init(uint64_t appId) {
    DiscordEventHandlers handlers{};

    handlers.ready = [](const DiscordUser* user) {
        std::fprintf(stderr, "[discord] ready, logged in as %s\n", user->username);
    };
    handlers.errored = [](int code, const char* msg) {
        std::fprintf(stderr, "[discord] error %d: %s\n", code, msg);
    };
    handlers.disconnected = [](int code, const char* msg) {
        std::fprintf(stderr, "[discord] disconnected %d: %s\n", code, msg);
    };

    Discord_Initialize(std::to_string(appId).c_str(), &handlers, 1, nullptr);
}

void DiscordManager::update() {
    Discord_RunCallbacks();
}

void DiscordManager::setPresence(const std::string& details,
                                 const std::string& state) {
    DiscordRichPresence presence{};
    presence.details = details.c_str();
    presence.state   = state.c_str();

    Discord_UpdatePresence(&presence);
}

void DiscordManager::clearPresence() {
    Discord_ClearPresence();
}

void DiscordManager::cleanup() {
    Discord_Shutdown();
}