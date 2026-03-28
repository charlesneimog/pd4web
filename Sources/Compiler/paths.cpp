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

#if defined(_WIN32)
    m_Emcmake = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emcmake.bat";
    m_Emcc = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emcc.bat";
    m_Emconfigure = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emconfigure.bat";
    m_Emmake = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emmake.bat";
    m_Ninja = m_Pd4WebRoot / "emsdk" / "ninja" / "git-release_64bit" / "bin" / "ninja.exe";
    m_Clang = m_Pd4WebRoot / "emsdk" / "upstream" / "bin" / "clang.exe";
#else
    m_Emcmake = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emcmake";
    m_Emcc = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emcc";
    m_Emconfigure = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emconfigure";
    m_Emmake = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emmake";
    m_Ninja = m_Pd4WebRoot / "emsdk" / "ninja" / "git-release_64bit" / "bin" / "ninja";
    m_Clang = m_Pd4WebRoot / "emsdk" / "upstream" / "bin" / "clang";
#endif

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
    fs::path envemscripten = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / ".emscripten";
    if (!fs::exists(envemscripten)) {
        fs::create_directories(envemscripten.parent_path());
        std::ofstream out(envemscripten);
        out << "LLVM_ROOT = r'" << (m_Pd4WebRoot / "emsdk" / "upstream" / "bin").string() << "'\n";
        out << "NODE_JS = r'" << m_NodeJs.string() << "'\n";
        out << "BINARYEN_ROOT = r'" << (m_Pd4WebRoot / "emsdk" / "upstream").string() << "'\n";
#if defined(_WIN32)
        out << "EMSDK_PY = r'" << m_PythonWindows.string() << "'\n";
#endif

#if defined(__APPLE__)
        fs::path envemscripten = m_Pd4WebRoot / "emsdk" / "python";
        // list all files inside envemscripten, them inside each folder, check if there is a
        // bin/python3 or bin/python3.13, and if there is create a symlink to m_PythonWindows
        fs::path emscriptenPython;
        for (const auto &entry : fs::directory_iterator(envemscripten)) {
            if (fs::is_directory(entry.path())) {
                fs::path test1 = entry.path() / "bin" / "python3";
                if (fs::exists(test1)) {
                    emscriptenPython = entry.path() / "bin" / "python3.13";
                }
                fs::path test2 = entry.path() / "bin" / "python3.13";
                if (fs::exists(test2)) {
                    emscriptenPython = test2;
                }
            }
        }

        // if emscriptenPython is not empty, save it on EMSDK_PY
        if (!emscriptenPython.empty()) {
            out << "EMSDK_PY = r'" << emscriptenPython.string() << "'\n";
        } else {
            print("Failed to find Python for Emscripten, if you have it installed you can set it "
                  "manually on " +
                      envemscripten.string(),
                  Pd4WebLogLevel::PD4WEB_WARNING);
        }
#endif

        out.close();
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::cmdInstallEmsdk() {
    PD4WEB_LOGGER();

#if defined(_WIN32)
    print("Installing emsdk, this can take a LONG some time.", Pd4WebLogLevel::PD4WEB_LOG2);
    std::vector<std::string> cmd = {"install", EMSDK_VERSION};
    int result = execProcess(m_EmsdkInstaller.string(), cmd);
    if (result != 0) {
        print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    print("Installing Node.js, this take some time", Pd4WebLogLevel::PD4WEB_LOG2);
    cmd = {"install", "node-22.16.0-64bit"};
    result = execProcess(m_EmsdkInstaller.string(), cmd);
    if (result != 0) {
        print("Failed to install Node.js", Pd4WebLogLevel::PD4WEB_ERROR);
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

    // Install ninja, to avoid mac warnings
    print("Installing ninja, this can take a some time.", Pd4WebLogLevel::PD4WEB_LOG2);
    std::vector<std::string> ninjacmd = {"install", EMSDK_VERSION};
    result = execProcess(m_EmsdkInstaller, ninjacmd);
    if (result != 0) {
        print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    print("");
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::getNode() {
    PD4WEB_LOGGER();
    fs::path nodePath = m_Pd4WebRoot / "emsdk/node";
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
    fs::path ninjaBin = m_Pd4WebRoot / "bin/ninja-linux";
#elif defined(_WIN32)
    fs::path ninjaBin = m_Pd4WebRoot / "bin/ninja.exe";
#elif defined(__APPLE__)
    fs::path ninjaBin = m_Pd4WebRoot / "bin/ninja-mac";
#else
    std::cerr << "Unsupported platform for Ninja binary." << std::endl;
    return false;
#endif

    print("Checking Ninja binary at: " + ninjaBin.string(), Pd4WebLogLevel::PD4WEB_LOG2);

    if (!fs::exists(ninjaBin)) {
        print("File not found: " + ninjaBin.string(), Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

#if defined(__linux__) || defined(__APPLE__)
    fs::path link = m_Pd4WebRoot / "bin/ninja";
    if (!fs::exists(link)) {
        print("Creating symlink for Ninja at: " + link.string(), Pd4WebLogLevel::PD4WEB_LOG2);
        fs::create_symlink(ninjaBin, link);
    }
    ninjaBin = link;

    std::error_code permEc;
    fs::permissions(ninjaBin,
                    fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add, permEc);
    if (permEc) {
        print("Failed to update permissions for Ninja binary: " + permEc.message(),
              Pd4WebLogLevel::PD4WEB_WARNING);
    }
#endif

    m_Ninja = ninjaBin;
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::getCmakeBinary() {
    PD4WEB_LOGGER();

    std::vector<fs::path> candidates;
#if defined(__linux__)
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "bin" / "cmake-linux");
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "cmake-linux");
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "bin" / "cmake");
#elif defined(__APPLE__)
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "bin" / "cmake-mac");
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "cmake-mac");
    // Older/alternate bundles might ship unrenamed.
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "bin" / "cmake");
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "cmake");
#elif defined(_WIN32)
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "bin" / "cmake.exe");
    candidates.push_back(m_Pd4WebRoot / "bin" / "cmake" / "cmake.exe");
#else
    print("Unsupported platform for CMake binary.", Pd4WebLogLevel::PD4WEB_ERROR);
    return false;
#endif

    fs::path cmakeBinary;
    for (const auto &candidate : candidates) {
        if (fs::exists(candidate)) {
            cmakeBinary = candidate;
            break;
        }
    }

    if (cmakeBinary.empty()) {
        std::string msg = "CMake binary not found. Tried:";
        for (const auto &candidate : candidates) {
            msg += "\n  - " + candidate.string();
        }
        fs::path cmakeRoot = m_Pd4WebRoot / "bin" / "cmake";
        if (fs::exists(cmakeRoot)) {
            msg += "\nCMake folder exists at: " + cmakeRoot.string();
        } else {
            msg += "\nCMake folder missing at: " + cmakeRoot.string();
        }
        print(msg, Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

#if defined(__linux__) || defined(__APPLE__)
    fs::path link = m_Pd4WebRoot / "bin" / "cmake" / "bin" / "cmake";
    if (!fs::exists(link)) {
        fs::create_symlink(cmakeBinary, link);
    }
    cmakeBinary = link;

    std::error_code permEc;
    fs::permissions(cmakeBinary,
                    fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add, permEc);
    if (permEc) {
        print("Failed to update permissions for CMake binary: " + permEc.message(),
              Pd4WebLogLevel::PD4WEB_WARNING);
    }
#endif

    m_Cmake = cmakeBinary;
    return true;
}

// ─────────────────────────────────────
std::string Pd4Web::getEmsdkPath() {
    PD4WEB_LOGGER();
    std::string path = (m_Pd4WebRoot / "emsdk/emsdk").string();
#if defined(_WIN32)
    path += ".bat";
#endif
    return path;
}
