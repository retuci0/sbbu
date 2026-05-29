#include "core/Options.h"

#include <fstream>


void Options::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    auto writeKey = [&](const char* name, SDL_KeyCode code) {
        file << name << "=" << static_cast<int>(code) << "\n";
    };

    writeKey("keyP1Left",     keyP1Left);
    writeKey("keyP1Right",    keyP1Right);
    writeKey("keyP1Down",     keyP1Down);
    writeKey("keyP1Jump",     keyP1Jump);
    writeKey("keyP1Shoot",    keyP1Shoot);
    writeKey("keyP1Melee",    keyP1Melee);
    writeKey("keyP1Special",  keyP1Special);
    writeKey("keyP1Shield",   keyP1Shield);

    writeKey("keyP2Left",     keyP2Left);
    writeKey("keyP2Right",    keyP2Right);
    writeKey("keyP2Down",     keyP2Down);
    writeKey("keyP2Jump",     keyP2Jump);
    writeKey("keyP2Shoot",    keyP2Shoot);
    writeKey("keyP2Melee",    keyP2Melee);
    writeKey("keyP2Special",  keyP2Special);
    writeKey("keyP2Shield",   keyP2Shield);

    writeKey("keyPause",      keyPause);
    writeKey("keyQuit",       keyQuit);
    writeKey("keyDebug",      keyDebug);
    writeKey("keyFullscreen", keyFullscreen);

    file << "sfxVolume="   << sfxVolume   << "\n";
    file << "musVolume="   << musVolume   << "\n";
    file << "debug="       << (debug ? 1 : 0) << "\n";
    file << "fpsCap="      << fpsCap      << "\n";
    file << "vsync="       << vsync       << "\n";
    file << "fullscreen="  << fullscreen  << "\n";

    file.close();
}

void Options::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return; // no config yet, keep defaults

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto sep = line.find('=');
        if (sep == std::string::npos) continue;

        std::string key = line.substr(0, sep);
        std::string val = line.substr(sep + 1);

        auto parseKey = [&](SDL_KeyCode& target, const char* expectedKey) {
            if (key == expectedKey) target = static_cast<SDL_KeyCode>(std::stoi(val));
        };
        
        parseKey(keyP1Left,     "keyP1Left");
        parseKey(keyP1Right,    "keyP1Right");
        parseKey(keyP1Down,     "keyP1Down");
        parseKey(keyP1Jump,     "keyP1Jump");
        parseKey(keyP1Shoot,    "keyP1Shoot");
        parseKey(keyP1Melee,    "keyP1Melee");
        parseKey(keyP1Special,  "keyP1Special");
        parseKey(keyP1Shield,   "keyP1Shield");

        parseKey(keyP2Left,     "keyP2Left");
        parseKey(keyP2Right,    "keyP2Right");
        parseKey(keyP2Down,     "keyP2Down");
        parseKey(keyP2Jump,     "keyP2Jump");
        parseKey(keyP2Shoot,    "keyP2Shoot");
        parseKey(keyP2Melee,    "keyP2Melee");
        parseKey(keyP2Special,  "keyP2Special");
        parseKey(keyP2Shield,   "keyP2Shield");

        parseKey(keyPause,      "keyPause");
        parseKey(keyQuit,       "keyQuit");
        parseKey(keyDebug,      "keyDebug");
        parseKey(keyFullscreen, "keyFullscreen");

        if (key == "sfxVolume")   sfxVolume   = std::stof(val);
        if (key == "musVolume")   musVolume   = std::stof(val);
        if (key == "debug")       debug       = (std::stoi(val) != 0);

        if (key == "fpsCap")      fpsCap      = std::stoi(val);
        if (key == "vsync")       vsync       = (std::stoi(val) != 0);
        if (key == "fullscreen")  fullscreen  = (std::stoi(val) != 0);
    }
}