#include "pd4web_compiler.hpp"

// ─────────────────────────────────────
bool Pd4Web::cmdExecute(std::string cmd) {
    PD4WEB_LOGGER();
#ifdef _WIN32
#error "Command execution on Windows is not implemented yet."
#else
    int ret = system(cmd.c_str());
    if (ret != 0) {
        return false;
    }
#endif

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
