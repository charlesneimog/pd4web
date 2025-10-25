#include "pd4web_compiler.hpp"

#include <filesystem>
#include <fstream>

#if defined(__APPLE__)
#include <sys/xattr.h>
#endif

bool Pd4Web::initPaths() {
    PD4WEB_LOGGER();
    m_EmsdkInstaller = getEmsdkPath();
    if (m_EmsdkInstaller.empty()) {
        return false;
    }

    m_Emcmake = m_Pd4WebRoot + "emsdk/upstream/emscripten/emcmake";
    m_Emcc = m_Pd4WebRoot + "emsdk/upstream/emscripten/emcc";
    m_Emconfigure = m_Pd4WebRoot + "emsdk/upstream/emscripten/emconfigure";
    m_Emmake = m_Pd4WebRoot + "emsdk/upstream/emscripten/emmake";
    m_Ninja = m_Pd4WebRoot + "emsdk/ninja/git-release_64bit/bin/ninja";
    m_Clang = m_Pd4WebRoot + "emsdk/upstream/bin/clang";

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::checkAllPaths() {
    PD4WEB_LOGGER();
    print("Checking emscripten paths", Pd4WebLogLevel::PD4WEB_LOG2, 2);

    bool ok = std::filesystem::exists(m_Emcmake);
    if (!ok) {
        print("emcmake not found", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    ok = std::filesystem::exists(m_Emcc);
    if (!ok) {
        print("emcc not found", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    ok = std::filesystem::exists(m_Emconfigure);
    if (!ok) {
        print("emconfigure not found", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    ok = std::filesystem::exists(m_Emmake);
    if (!ok) {
        print("emmake not found", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // Check cmake
    ok = getCmakeBinary();
    if (!ok) {
        print("Failed to get Cmake Binary", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // Check Ninja
    ok = getNinja();
    if (!ok) {
        print("Failed to get Ninja", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // Check if all paths are set
    fs::path envemscripten = m_Pd4WebRoot + "emsdk/upstream/emscripten/.emscripten";
    if (!fs::exists(envemscripten)) {
        fs::remove(envemscripten);
        std::ofstream out(envemscripten);
        out << "LLVM_ROOT = r'" << (m_Pd4WebRoot + "emsdk/upstream/bin") << "'\n";
        out << "BINARYEN_ROOT = r'" << (m_Pd4WebRoot + "emsdk/upstream/binaryen") << "'\n";

        // find node
        fs::path nodePath = m_Pd4WebRoot + "emsdk/node";
        bool nodeFound = false;
        for (const auto &entry : fs::directory_iterator(nodePath)) {
            if (fs::is_directory(entry.path())) {
                fs::path nodeBin = entry.path() / "bin" / "node";
                if (fs::exists(nodeBin)) {
                    out << "NODE_JS = [r'" << nodeBin.string() << "']\n";
                    nodeFound = true;
                    break;
                }
            }
        }
        if (!nodeFound) {
            print("Node.js not found in emsdk. Please install it using '" + m_EmsdkInstaller +
                      "install node-<version>'",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        out.close();
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::cmdInstallEmsdk() {
    PD4WEB_LOGGER();

    print("Installing emsdk, this can take a LONG some time.", Pd4WebLogLevel::PD4WEB_LOG2);
    std::vector<std::string> cmd = {"install", EMSDK_VERSION};
    int result = execProcess(m_EmsdkInstaller, cmd);
    if (result != 0) {
        print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    print("");
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::getNinja() {
    if (fs::exists(m_Ninja)) {
        return true;
    }
#if defined(__linux__)
    std::string ninjaBin = m_Pd4WebRoot + "/bin/ninja";
#elif defined(_WIN32)
    std::string ninjaBin = m_Pd4WebRoot + "/bin/ninja.exe";
#elif defined(__APPLE__)
    std::string ninjaBin = m_Pd4WebRoot + "/bin/ninja";
#else
    std::cerr << "Unsupported platform for Ninja binary." << std::endl;
#endif

    if (fs::exists(ninjaBin)) {
#if defined(__linux__) || defined(__APPLE__)
        fs::permissions(ninjaBin,
                        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                        fs::perm_options::add);
#if defined(__APPLE__)
        // Remove macOS quarantine attribute if present
        int ok = removexattr(ninjaBin.c_str(), "com.apple.quarantine", 0);
        if (ok != 0 && errno != ENOATTR && errno != ENODATA) {
            std::cerr << "Failed to remove macOS quarantine attribute from Ninja.\n";
            return false;
        }
#endif
#endif
        m_Ninja = ninjaBin;
        return true;
    }

    return false;
}

// ─────────────────────────────────────
bool Pd4Web::getCmakeBinary() {
    PD4WEB_LOGGER();

    // Determine platform-specific CMake binary path
    std::string cmakeBinary;
#if defined(__linux__) || defined(__APPLE__)
    cmakeBinary = m_Pd4WebRoot + "/bin/cmake/bin/cmake";
#elif defined(_WIN32)
    cmakeBinary = m_Pd4WebRoot + "/bin/cmake/bin/cmake.exe";
#else
    print("Unsupported platform for CMake binary.", Pd4WebLogLevel::PD4WEB_ERROR);
    return false;
#endif
    if (fs::exists(cmakeBinary)) {
#if defined(__linux__) || defined(__APPLE__)
        fs::permissions(cmakeBinary,
                        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                        fs::perm_options::add);
#if defined(__APPLE__)
        // Remove macOS quarantine attribute if present
        int ok = removexattr(cmakeBinary.c_str(), "com.apple.quarantine", 0);
        if (ok != 0 && errno != ENOATTR && errno != ENODATA) {
            std::cerr << "Failed to remove macOS quarantine attribute from Ninja.\n";
            return false;
        }
#endif
#endif

        m_Cmake = cmakeBinary;
        return true;
    }
    return false;
}

// ─────────────────────────────────────
std::string Pd4Web::getEmsdkPath() {
    PD4WEB_LOGGER();
    std::string path = m_Pd4WebRoot + "emsdk/emsdk";
#if defined(_WIN32)
    path += ".bat";
#endif
    return path;
}
