#include "pd4web_compiler.hpp"

int main(int argc, char *argv[]) {
#if defined(__APPLE__) || defined(__linux__)
    std::string home = std::getenv("HOME");
    std::filesystem::path pd4webHome = std::filesystem::path(home) / ".local" / "share" / "pd4web";
    std::filesystem::create_directories(pd4webHome);
#else
    std::string appdata = std::getenv("APPDATA");
    std::filesystem::path pd4webHome = std::filesystem::path(appdata) / "pd4web";
    std::filesystem::create_directories(pd4webHome);
#endif

    // process
    Pd4Web pd4web(pd4webHome.string());
    pd4web.parseArgs(argc, argv);

    pd4web.setPd4WebFilesFolder("/home/neimog/Documents/Git/pd4web/Sources/Pd4Web");
    pd4web.init();
    pd4web.processPatch();
    return 0;
}
