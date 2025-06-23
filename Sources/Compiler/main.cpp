#include "pd4web.hpp"

int main(int argc, char *argv[]) {
#if defined(__APPLE__) || defined(__linux__)
    std::string home = std::getenv("HOME");
    std::filesystem::path pd4webHome = std::filesystem::path(home) / ".local" / "share" / "pd4web";
    std::filesystem::create_directories(pd4webHome);
#else
#endif

    // process
    Pd4Web pd4web(pd4webHome);
    pd4web.parseArgs(argc, argv);
    pd4web.processPatch();
    return 0;
}
