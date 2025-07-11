#include "pd4web_compiler.hpp"

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
    // TODO: do for windows
#else
    print("Installing emsdk, this will take some time", LOG2);
    std::string cmd = "bash " + m_EmsdkInstaller + " install " + EMSDK_VERSION;
    if (!cmdExecute(cmd)) {
        return false;
    }
    cmd = "bash " + m_EmsdkInstaller + " install " + "ninja-git-release-64bit";
    return cmdExecute(cmd);
#endif
    return true;
}
