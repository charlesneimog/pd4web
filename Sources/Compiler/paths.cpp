#include "pd4web_compiler.hpp"

#include <filesystem>
#include <fstream>
#include <system_error>
#if defined(__APPLE__)
#include <cerrno>
#endif

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

    // Check Node.js
    ok = getNode();
    if (!ok) {
        print("Failed to get Node.js", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // Check if all paths are set
    fs::path envemscripten = m_Pd4WebRoot + "emsdk/upstream/emscripten/.emscripten";
    if (!fs::exists(envemscripten)) {
        fs::remove(envemscripten);
        std::ofstream out(envemscripten);
        out << "LLVM_ROOT = r'" << (m_Pd4WebRoot + "emsdk/upstream/bin") << "'\n";
        out << "BINARYEN_ROOT = r'" << (m_Pd4WebRoot + "emsdk/upstream/binaryen") << "'\n";
#if defined(_WIN32)
        out << "EMSDK_PY = r'" << m_PythonWindows << "'\n";
        out << "SSL_CERT_FILE = r'" << getCertFile() << "'\n";
        out << "NODE_JS = r'" << m_NodeJs << "'\n";
#endif
        out.close();
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::cmdInstallEmsdk() {
    PD4WEB_LOGGER();

#if defined(_WIN32)
    print("Installing Node.js, this take some time", Pd4WebLogLevel::PD4WEB_LOG2);
    std::vector<std::string> cmd = {"install", "node-22.16.0-64bits"};
    int result = execProcess(m_EmsdkInstaller, cmd);
    if (result != 0) {
        print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
#else
    print("Installing emsdk, this can take a LONG some time.", Pd4WebLogLevel::PD4WEB_LOG2);
    std::vector<std::string> cmd = {"install", EMSDK_VERSION};
    int result = execProcess(m_EmsdkInstaller, cmd);
    if (result != 0) {
        print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
#endif
    print("");
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::getNode() {
    PD4WEB_LOGGER();
    fs::path nodePath = m_Pd4WebRoot + "/emsdk/node";
    // list all folders inside nodePath, and find the first one that contains bin/node or
    // bin/node.exe
    for (const auto &entry : fs::directory_iterator(nodePath)) {
        if (fs::is_directory(entry.path())) {
            fs::path nodeBin;
#if defined(_WIN32)
            nodeBin = entry.path() / "bin" / "node.exe";
#else
            nodeBin = entry.path() / "bin" / "node";
#endif
            if (fs::exists(nodeBin)) {
                m_NodeJs = nodeBin.string();
                return true;
            }
        }
    }

    return false;
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
    return false;
#endif

    if (!fs::exists(ninjaBin)) {
        return false;
    }

#if defined(__linux__) || defined(__APPLE__)
    std::error_code permEc;
    fs::permissions(ninjaBin,
                    fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add, permEc);
    if (permEc) {
        print("Failed to update permissions for Ninja binary: " + permEc.message(),
              Pd4WebLogLevel::PD4WEB_WARNING);
    }
#if defined(__APPLE__)
    if (removexattr(ninjaBin.c_str(), "com.apple.quarantine", 0) != 0 && errno != ENOATTR &&
        errno != ENODATA) {
        std::cerr << "Failed to remove macOS quarantine attribute from the Ninja binary." << '\n';
        return false;
    }
#endif
#endif

    m_Ninja = ninjaBin;
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::getCmakeBinary() {
    PD4WEB_LOGGER();

    std::string cmakeBinary;
#if defined(__linux__) || defined(__APPLE__)
    cmakeBinary = m_Pd4WebRoot + "/bin/cmake/bin/cmake";
#elif defined(_WIN32)
    cmakeBinary = m_Pd4WebRoot + "/bin/cmake/bin/cmake.exe";
#else
    print("Unsupported platform for CMake binary.", Pd4WebLogLevel::PD4WEB_ERROR);
    return false;
#endif

    if (!fs::exists(cmakeBinary)) {
        return false;
    }

#if defined(__linux__) || defined(__APPLE__)
    std::error_code permEc;
    fs::permissions(cmakeBinary,
                    fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add, permEc);
    if (permEc) {
        print("Failed to update permissions for CMake binary: " + permEc.message(),
              Pd4WebLogLevel::PD4WEB_WARNING);
    }
#if defined(__APPLE__)
    if (removexattr(cmakeBinary.c_str(), "com.apple.quarantine", 0) != 0 && errno != ENOATTR &&
        errno != ENODATA) {
        std::cerr << "Failed to remove macOS quarantine attribute from the CMake binary." << '\n';
        return false;
    }
#endif
#endif

    m_Cmake = cmakeBinary;
    return true;
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
