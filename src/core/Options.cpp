#include "core/Options.h"

#include <exception>
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
    writeKey("keyP1Dash",     keyP1Dash);
    writeKey("keyP1Grapple",  keyP1Grapple);

    writeKey("keyP2Left",     keyP2Left);
    writeKey("keyP2Right",    keyP2Right);
    writeKey("keyP2Down",     keyP2Down);
    writeKey("keyP2Jump",     keyP2Jump);
    writeKey("keyP2Shoot",    keyP2Shoot);
    writeKey("keyP2Melee",    keyP2Melee);
    writeKey("keyP2Special",  keyP2Special);
    writeKey("keyP2Shield",   keyP2Shield);
    writeKey("keyP2Dash",     keyP2Dash);
    writeKey("keyP2Grapple",  keyP2Grapple);

    writeKey("keyPause",      keyPause);
    writeKey("keyQuit",       keyQuit);
    writeKey("keyDebug",      keyDebug);
    writeKey("keyFullscreen", keyFullscreen);
    writeKey("keyScreenshot", keyScreenshot);
    writeKey("keyCheats",     keyCheats);

    file << "sfxVolume="   << sfxVolume   << "\n";
    file << "musVolume="   << musVolume   << "\n";
    file << "debug="       << debug       << "\n";
    file << "fpsCap="      << fpsCap      << "\n";
    file << "vsync="       << vsync       << "\n";
    file << "fullscreen="  << fullscreen  << "\n";
    file << "particles="   << particles   << "\n";
    file.close();
}

void Options::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return; // no config yet, keep defaults

    std::string line;
    while (std::getline(file, line)) {
        // ignore empty lines or comments
        if (line.empty() || line[0] == '#') continue;

        auto sep = line.find('=');
        if (sep == std::string::npos) continue;

        std::string key = line.substr(0, sep);
        std::string val = line.substr(sep + 1);

        auto parseKey = [&](SDL_KeyCode& target, const char* expectedKey) {
            try {
                if (key == expectedKey) {
                    target = static_cast<SDL_KeyCode>(std::stoi(val));
                }
            } catch (const std::exception& e) {
                // empty catch block
            }
        };
        
        parseKey(keyP1Left,     "keyP1Left");
        parseKey(keyP1Right,    "keyP1Right");
        parseKey(keyP1Down,     "keyP1Down");
        parseKey(keyP1Jump,     "keyP1Jump");
        parseKey(keyP1Shoot,    "keyP1Shoot");
        parseKey(keyP1Melee,    "keyP1Melee");
        parseKey(keyP1Special,  "keyP1Special");
        parseKey(keyP1Shield,   "keyP1Shield");
        parseKey(keyP1Dash,     "keyP1Dash");
        parseKey(keyP1Grapple,  "keyP1Grapple");

        parseKey(keyP2Left,     "keyP2Left");
        parseKey(keyP2Right,    "keyP2Right");
        parseKey(keyP2Down,     "keyP2Down");
        parseKey(keyP2Jump,     "keyP2Jump");
        parseKey(keyP2Shoot,    "keyP2Shoot");
        parseKey(keyP2Melee,    "keyP2Melee");
        parseKey(keyP2Special,  "keyP2Special");
        parseKey(keyP2Shield,   "keyP2Shield");
        parseKey(keyP2Dash,     "keyP2Dash");
        parseKey(keyP2Grapple,  "keyP2Grapple");

        parseKey(keyPause,      "keyPause");
        parseKey(keyQuit,       "keyQuit");
        parseKey(keyDebug,      "keyDebug");
        parseKey(keyFullscreen, "keyFullscreen");
        parseKey(keyScreenshot, "keyScreenshot");
        parseKey(keyCheats,     "keyCheats");

        auto getFloatValue = [=](float& v) {
            try {
                v = std::stof(val);
            } catch (const std::exception& e) {
                // empty catch block
            }
        };

        auto getBoolValue = [=](bool& v) {
            try {
                v = (std::stoi(val) != 0);
            } catch (const std::exception& e) {
                // empty catch block
            }
        };

        auto getIntValue = [=](int& v) {
            try {
                v = std::stoi(val);
            } catch (const std::exception& e) {
                // empty catch block
            }
        };

        if (key == "sfxVolume")   getFloatValue(sfxVolume);
        if (key == "musVolume")   getFloatValue(musVolume);
        if (key == "fpsCap")      getIntValue(fpsCap);
        if (key == "debug")       getBoolValue(debug);
        if (key == "vsync")       getBoolValue(vsync);
        if (key == "fullscreen")  getBoolValue(fullscreen);
        if (key == "particles")   getBoolValue(particles);
    }
}