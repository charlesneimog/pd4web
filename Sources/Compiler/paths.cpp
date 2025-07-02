#include "pd4web.hpp"
#include <filesystem>

// ─────────────────────────────────────
bool Pd4Web::initPaths() {
    print(__FUNCTION__, Pd4WebLogLevel::VERBOSE);
    m_EmsdkInstaller = getEmsdkPath();
    if (m_EmsdkInstaller.empty()) {
        return false;
    }

    m_Emcmake = m_Pd4WebRoot + "emsdk/upstream/emscripten/emcmake";
    m_Emcc = m_Pd4WebRoot + "emsdk/upstream/emscripten/emcc";
    m_Emconfigure = m_Pd4WebRoot + "emsdk/upstream/emscripten/emconfigure";
    m_Emmake = m_Pd4WebRoot + "emsdk/upstream/emscripten/emmake";
    m_Ninja = m_Pd4WebRoot + "emsdk/ninja/git-release_64bit/bin/ninja";

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::checkAllPaths() {
    print(__FUNCTION__, Pd4WebLogLevel::VERBOSE);
    // check if m_Emcmake exists

    print("Checking emscripten paths", Pd4WebLogLevel::LOG2, 2);

    bool ok = std::filesystem::exists(m_Emcmake);
    if (!ok) {
        print("emcmake not found", Pd4WebLogLevel::ERROR);
        return false;
    }
    ok = std::filesystem::exists(m_Emcc);
    if (!ok) {
        print("emcc not found", Pd4WebLogLevel::ERROR);
        return false;
    }
    ok = std::filesystem::exists(m_Emconfigure);
    if (!ok) {
        print("emconfigure not found", Pd4WebLogLevel::ERROR);
        return false;
    }
    ok = std::filesystem::exists(m_Emmake);
    if (!ok) {
        print("emmake not found", Pd4WebLogLevel::ERROR);
        return false;
    }

    ok = getCmakeBinary();
    if (!ok) {
        print("Failed to get Cmake Binary", Pd4WebLogLevel::ERROR);
        return false;
    }

    return true;
}
// ─────────────────────────────────────
bool Pd4Web::getCmakeBinary() {
    std::string cmakeBinary = m_Pd4WebRoot + "cmake/bin/cmake";
#if defined(_WIN32)
    cmakeBinary += ".exe";
#endif

    if (fs::exists(cmakeBinary)) {
        m_Cmake = cmakeBinary;
        return true;
    }

    std::string asset = "";
#if defined(__linux__)
#if defined(__x86_64__)
    asset = "cmake-4.0.3-linux-x86_64.tar.gz";
#elif defined(__aarch64__)
    asset = "cmake-4.0.3-linux-aarch64.tar.gz";
#endif
#elif defined(__APPLE__)
    asset = "cmake-4.0.3-macos-universal.tar.gz";
#elif defined(_WIN32)
#if defined(_M_X64)
    asset = "cmake-4.0.3-windows-x86_64.zip";
#elif defined(_M_ARM64)
    asset = "cmake-4.0.3-windows-arm64.zip";
#endif
#endif

    if (asset.empty()) {
        std::cerr << "Unsupported platform." << std::endl;
        exit(-1);
    }

    std::string url = "/Kitware/CMake/releases/download/v4.0.3/" + asset;

    print("Downloading " + url, Pd4WebLogLevel::LOG2);
    httplib::SSLClient cli("github.com");
    cli.set_follow_location(true);

    auto res = cli.Get(url.c_str());
    if (!res || res->status != 200) {
        print("Failed to get " + url + ". Code: " + std::to_string(res->status),
              Pd4WebLogLevel::ERROR);
        return false;
    }

    std::string cmakeOutput = m_Pd4WebRoot + asset;
    std::ofstream ofs(cmakeOutput, std::ios::binary);
    ofs << res->body;

#if defined(_WIN32)
    std::string output_dir = m_Pd4WebRoot + "cmake";
    std::string zip_path = cmakeOutput;
    std::string cmd = "powershell -Command \""
                      "if (-Not (Test-Path -Path '" +
                      output_dir + "')) { New-Item -ItemType Directory -Path '" + output_dir +
                      "' } ; "
                      "Expand-Archive -Path '" +
                      zip_path + "' -DestinationPath '" + output_dir + "' -Force\"";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Erro ao extrair arquivo zip no Windows" << std::endl;
        exit(-1);
    }
    fs::remove(cmakeOutput);

    if (fs::exists(cmakeBinary)) {
        m_Cmake = cmakeBinary;
    } else {
        print("Empty cmake path", Pd4WebLogLevel::ERROR);
        exit(-1);
    }

#else
    std::string cmd = "mkdir -p " + (m_Pd4WebRoot + "cmake") +
                      " && tar --strip-components=1 -xzf " + cmakeOutput + " -C " +
                      (m_Pd4WebRoot + "cmake");
    int ret = std::system(cmd.c_str());

    if (ret != 0) {
        std::cerr << "Erro ao extrair arquivo cmake" << std::endl;
        exit(-1);
    }

    fs::remove(cmakeOutput);

    if (fs::exists(cmakeBinary)) {
        m_Cmake = cmakeBinary;
    } else {
        print("Empty cmake path", Pd4WebLogLevel::ERROR);
        exit(-1);
    }

    return true;
#endif
}

// ─────────────────────────────────────
std::string Pd4Web::getEmsdkPath() {
    print(__FUNCTION__, Pd4WebLogLevel::VERBOSE);
    std::string path = m_Pd4WebRoot + "emsdk/emsdk";
    print(path, Pd4WebLogLevel::VERBOSE);

#ifdef _WIN32
    path += ".bat";
#else
#endif
    return path;
}
