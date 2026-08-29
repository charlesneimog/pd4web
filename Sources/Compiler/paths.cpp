#include "pd4web_compiler.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
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
    m_Emcmake = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emcmake.exe";
    m_Emcc = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emcc.exe";
    m_Emconfigure = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emconfigure.exe";
    m_Emmake = m_Pd4WebRoot / "emsdk" / "upstream" / "emscripten" / "emmake.exe";
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
        out.close();
    }

    // Keep existing Emscripten installations in sync as well. Older pd4web versions wrote the
    // unsupported EMSDK_PY setting here, so replace either spelling with Emscripten's PYTHON key.
    std::ifstream configInput(envemscripten);
    if (!configInput) {
        print("Failed to read Emscripten config: " + envemscripten.string(),
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    std::ostringstream updatedConfig;
    const std::string pythonSetting = "PYTHON = r'" + m_PythonInterpreter.string() + "'";
    bool wrotePythonSetting = false;
    std::string configLine;
    while (std::getline(configInput, configLine)) {
        const size_t firstCharacter = configLine.find_first_not_of(" \t");
        const bool isPythonSetting = firstCharacter != std::string::npos &&
                                     (configLine.compare(firstCharacter, 8, "PYTHON =") == 0 ||
                                      configLine.compare(firstCharacter, 10, "EMSDK_PY =") == 0);
        if (isPythonSetting) {
            if (!wrotePythonSetting) {
                updatedConfig << pythonSetting << '\n';
                wrotePythonSetting = true;
            }
            continue;
        }
        updatedConfig << configLine << '\n';
    }
    configInput.close();

    if (!wrotePythonSetting) {
        updatedConfig << pythonSetting << '\n';
    }

    std::ofstream configOutput(envemscripten, std::ios::trunc);
    if (!configOutput) {
        print("Failed to update Emscripten config: " + envemscripten.string(),
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    configOutput << updatedConfig.str();
    configOutput.close();

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::cmdInstallEmsdk() {
    PD4WEB_LOGGER();

    print("Installing emsdk, this can take a LONG some time.", Pd4WebLogLevel::PD4WEB_LOG2);
    fs::path emsdkPy = m_Pd4WebRoot / "emsdk" / "emsdk.py";
    if (m_PythonInterpreter.empty() || !fs::exists(m_PythonInterpreter) || !fs::exists(emsdkPy)) {
        print("Cannot install emsdk without the validated Python interpreter and emsdk.py.",
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    std::vector<std::string> cmd = {emsdkPy.string(), "install", EMSDK_VERSION};
    int result = execProcess(m_PythonInterpreter.string(), cmd);

    if (result != 0) {
        print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

#if defined(_WIN32)
    print("Installing Node.js, this take some time", Pd4WebLogLevel::PD4WEB_LOG2);
    cmd = {emsdkPy.string(), "install", "node-22.16.0-64bit"};
    result = execProcess(m_PythonInterpreter.string(), cmd);

    if (result != 0) {
        print("Failed to install Node.js", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
#endif

    // // Install ninja, to avoid mac warnings
    // print("Installing ninja, this can take a some time.", Pd4WebLogLevel::PD4WEB_LOG2);
    // std::vector<std::string> ninjacmd = {"install", EMSDK_VERSION};
    // result = execProcess(m_EmsdkInstaller, ninjacmd);
    // if (result != 0) {
    //     print("Failed to install emsdk", Pd4WebLogLevel::PD4WEB_ERROR);
    //     return false;
    // }

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
