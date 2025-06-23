#include "pd4web.hpp"

// ─────────────────────────────────────
bool Pd4Web::cmdExecute(std::string cmd) {
#ifdef _WIN32
#else

    cmd += " > /dev/null 2>&1";
    int ret = system(cmd.c_str());
    if (ret != 0) {
        return false;
    }
#endif

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::cmdInstallEmsdk() {
#ifdef _WIN32
#else
    INFO("Installing emsdk, this will take some time");
    std::string cmd = "bash " + m_EmsdkInstaller + " install " + EMSDK_VERSION;
    return cmdExecute(cmd);
#endif
    return true;
}
