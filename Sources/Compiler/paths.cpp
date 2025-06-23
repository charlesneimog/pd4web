#include "pd4web.hpp"
#include <filesystem>

// ─────────────────────────────────────
bool Pd4Web::initPaths() {
    LOG(__FUNCTION__);
    m_EmsdkInstaller = getEmsdkPath();
    if (m_EmsdkInstaller.empty()) {
        return false;
    }

    m_Emcmake = m_Pd4WebRoot + "emsdk/upstream/emscripten/emcmake";
    m_Emcc = m_Pd4WebRoot + "emsdk/upstream/emscripten/emcc";
    m_Emconfigure = m_Pd4WebRoot + "emsdk/upstream/emscripten/emconfigure";
    m_Emmake = m_Pd4WebRoot + "emsdk/upstream/emscripten/emmake";

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::checkAllPaths() {
    LOG(__FUNCTION__);
    // check if m_Emcmake exists

    print("Checking emscripten paths", Pd4WebColor::GREEN);

    bool ok = std::filesystem::exists(m_Emcmake);
    if (!ok) {
        LOG("emcmake not found");
        return false;
    }
    ok = std::filesystem::exists(m_Emcc);
    if (!ok) {
        LOG("emcc not found");
        return false;
    }
    ok = std::filesystem::exists(m_Emconfigure);
    if (!ok) {
        LOG("emconfigure not found");
        return false;
    }
    ok = std::filesystem::exists(m_Emmake);
    if (!ok) {
        LOG("emmake not found");
        return false;
    }
    return true;
}

// ─────────────────────────────────────
std::string Pd4Web::getEmsdkPath() {
    LOG(__FUNCTION__);
    std::string path = m_Pd4WebRoot + "emsdk/emsdk";
    LOG(path);

#ifdef _WIN32
    path += ".bat";
#else
#endif
    return path;
}
