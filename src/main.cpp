#include "game.h"
#include <filesystem>
#include <fstream>
#include <ctime>
#include <cstdio>

static const char* CRASH_TEXT =
    "something went wrong!\n"
    "please report this crash log at https://github.com/retuci0/super-bert-bros-ultimate\n";

int main() {
    try {
        Game game;
        game.init();
        game.run();
        game.cleanup();
        return 0;
    } catch (const std::exception& e) {
        std::time_t now = std::time(nullptr);
        std::tm* tm     = std::localtime(&now);

        char filename[256];
        std::snprintf(filename, sizeof(filename),
                      "crash_logs/crash-%d-%d-%d_%d.%d.%d.txt",
                      1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
                      tm->tm_hour, tm->tm_min, tm->tm_sec);

        std::filesystem::create_directories("crash_logs");
        std::ofstream f(filename);
        if (f) {
            f << "CRASH REPORT "
              << 1900 + tm->tm_year << "-" << 1 + tm->tm_mon << "-" << tm->tm_mday
              << " at " << tm->tm_hour << ":" << tm->tm_min << "." << tm->tm_sec << "\n"
              << CRASH_TEXT << "\n"
              << e.what() << "\n";
        }

        // also print to stderr so it's visible in a terminal
        std::fprintf(stderr, "CRASH: %s\nWritten to %s\n", e.what(), filename);
        return 1;
    }
}