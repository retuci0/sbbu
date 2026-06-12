#include "misc/Discord.h"

#ifndef __EMSCRIPTEN__
    #include "discord_rpc.h"
    #include <cstdio>
#endif


void DiscordManager::init(uint64_t appId) {
#ifndef __EMSCRIPTEN__
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
#endif
}

void DiscordManager::update() {
#ifndef __EMSCRIPTEN__
    Discord_RunCallbacks();
#endif
}

void DiscordManager::setPresence(const std::string& details, const std::string& state) {
#ifndef __EMSCRIPTEN__
    DiscordRichPresence presence{};
    presence.details = details.c_str();
    presence.state   = state.c_str();

    Discord_UpdatePresence(&presence);
#endif
}

void DiscordManager::clearPresence() {
#ifndef __EMSCRIPTEN__
    Discord_ClearPresence();
#endif
}

void DiscordManager::cleanup() {
#ifndef __EMSCRIPTEN__
    Discord_Shutdown();
#endif
}