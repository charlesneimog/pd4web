#include "pd4web_compiler.hpp"

#include <filesystem>
#include <fstream>

#include <archive.h>
#include <archive_entry.h>

#if defined(__APPLE__)
#include <sys/xattr.h>
#endif

// ─────────────────────────────────────
bool extractZipToFolderStripTop(const std::string& zipPath, const std::string& destDir) {
    PD4WEB_LOGGER();
    namespace fs = std::filesystem;
    fs::create_directories(destDir);

    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    struct archive_entry* entry;

    // Enable ZIP support
    archive_read_support_format_zip(a);
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);

    if (archive_read_open_filename(a, zipPath.c_str(), 10240) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << "\n";
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* currentFile = archive_entry_pathname(entry);
        std::string pathStr(currentFile ? currentFile : "");

        // Strip the top-level folder
        size_t pos = pathStr.find('/');
        std::string strippedPath = (pos == std::string::npos) ? pathStr : pathStr.substr(pos + 1);

        if (strippedPath.empty()) {
            archive_read_data_skip(a);
            continue;
        }

        std::string fullOutputPath = destDir + "/" + strippedPath;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());

        if (archive_write_header(ext, entry) != ARCHIVE_OK) {
            std::cerr << "Warning: " << archive_error_string(ext) << "\n";
        } else {
            const void* buff;
            size_t size;
            la_int64_t offset;
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                archive_write_data_block(ext, buff, size, offset);
            }
        }
        archive_write_finish_entry(ext);
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return true;
}

// ─────────────────────────────────────
bool extractTarGzToFolderStripTop(const std::string& tarGzPath, const std::string& destDir) {
    PD4WEB_LOGGER();
    namespace fs = std::filesystem;
    fs::create_directories(destDir);

    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    struct archive_entry* entry;

    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);

    if (archive_read_open_filename(a, tarGzPath.c_str(), 10240) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << "\n";
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* currentFile = archive_entry_pathname(entry);
        std::string pathStr(currentFile);

        // Strip the top-level folder
        size_t pos = pathStr.find('/');
        std::string strippedPath = (pos == std::string::npos) ? pathStr : pathStr.substr(pos + 1);

        if (strippedPath.empty()) {
            archive_read_data_skip(a);
            continue;
        }

        std::string fullOutputPath = destDir + "/" + strippedPath;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());

        if (archive_write_header(ext, entry) != ARCHIVE_OK) {
            std::cerr << "Warning: " << archive_error_string(ext) << "\n";
        } else {
            const void* buff;
            size_t size;
            la_int64_t offset;
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                archive_write_data_block(ext, buff, size, offset);
            }
        }
        archive_write_finish_entry(ext);
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return true;
}

// ─────────────────────────────────────
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
        for (const auto& entry : fs::directory_iterator(nodePath)) {
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
            print("Node.js not found in emsdk. Please install it using '" + m_EmsdkInstaller + "install node-<version>'", Pd4WebLogLevel::PD4WEB_ERROR);
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
    std::string ninjaBin = m_Pd4WebRoot + "ninja/ninja";
#elif defined(_WIN32)
    std::string ninjaBin = m_Pd4WebRoot + "ninja.exe";
#elif defined(__APPLE__)
    std::string ninjaBin = m_Pd4WebRoot + "ninja/ninja";
#else
    std::cerr << "Unsupported platform for Ninja binary." << std::endl;
    exit(-1);
#endif

    if (fs::exists(ninjaBin)) {
        m_Ninja = ninjaBin;
        return true;
    }

std::string asset = "";
#if defined(__linux__)
#if defined(__x86_64__)
    asset = "ninja-linux.zip";
#elif defined(__aarch64__)
    asset = "ninja-linux-aarch64.zip";
#endif
#elif defined(__APPLE__)
    asset = "ninja-mac.zip";
#elif defined(_WIN32)
#if defined(_M_X64)
    asset = "ninja-win.zip";
#elif defined(_M_ARM64)
    asset = "ninja-winarm64.zip";
#endif
#endif
   
    if (asset.empty()) {
        std::cerr << "Unsupported platform." << std::endl;
        exit(-1);
    }

    std::string url = "/ninja-build/ninja/releases/download/v1.13.1/" + asset;
    print("Downloading " + url, Pd4WebLogLevel::PD4WEB_LOG2);

    httplib::SSLClient cli("github.com");
    cli.set_follow_location(true);
    cli.enable_server_certificate_verification(false);
    auto res = cli.Get(url.c_str());
    if (!res) {
        auto err = cli.get_openssl_verify_result();
        print("Failed to connect to GitHub to download CMake binary. Error code: " +
                  std::to_string(static_cast<int>(res.error())),
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    if (res->status != 200) {
        print("Failed to get " + url + ". HTTP code: " + std::to_string(res->status),
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    std::string ninjaOutput = m_Pd4WebRoot + asset;
    std::ofstream ofs(ninjaOutput, std::ios::binary);
    if (!ofs) {
        print("Failed to write to " + ninjaOutput, Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    ofs << res->body;
    if (!ofs.good()) {
        print("Failed to write to " + ninjaOutput, Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    ofs.close();


    std::string destDir = m_Pd4WebRoot + "ninja";
    if (!fs::exists(destDir)) {
        fs::create_directories(destDir);
    }

    if (!extractZipToFolderStripTop(ninjaOutput, destDir)) {
        print("Failed to extract CMake binary", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    } 

    fs::remove(ninjaOutput);
    if (fs::exists(ninjaBin)) {
        m_Ninja = ninjaBin;
    } else {
        print("Empty cmake path", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // chmod +x
    fs::permissions(
        m_Ninja,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add
    );

    std::cout << "Ninja binary permissions set to executable." << std::endl;
#if defined(__APPLE__)
    int ok = removexattr(m_Ninja.c_str(), "com.apple.quarantine", 0);
    if (ok != 0 && errno != ENOATTR && errno != ENODATA) {
        print("Failed to remove the quarantine attribute from the Ninja binary. "
            "Execution may fail. To allow it manually, open System Settings → Privacy & Security, "
            "scroll to the Security section, and click 'Open Anyway'.",
            Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    std::cout << "Quarantine attribute removed from the Ninja binary." << std::endl;
#endif

    print("Ninja binary is ready at " + m_Ninja, Pd4WebLogLevel::PD4WEB_LOG2);

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::getCmakeBinary() {
    PD4WEB_LOGGER();
#if defined(__linux__)
    std::string cmakeBinary = m_Pd4WebRoot + "cmake/bin/cmake";
#elif defined(_WIN32)
    std::string cmakeBinary = m_Pd4WebRoot + "cmake/bin/cmake"+".exe";
#elif defined(__APPLE__)
    std::string cmakeBinary = m_Pd4WebRoot + "cmake/CMake.app/Contents/bin/cmake";
#else
    std::cerr << "Unsupported platform for CMake binary." << std::endl;
    exit(-1);
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

    // TODO: Make this global and define it on Pd4Web constructor
    std::string url = "/Kitware/CMake/releases/download/v4.0.3/" + asset;
    print("Downloading " + url, Pd4WebLogLevel::PD4WEB_LOG2);

    httplib::SSLClient cli("github.com");
    cli.set_follow_location(true);
    cli.enable_server_certificate_verification(false);
    auto res = cli.Get(url.c_str());
    
    if (!res) {
        print("Failed to connect to GitHub to download CMake binary. Error code: " +
                  std::to_string(static_cast<int>(res.error())),
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    if (res->status != 200) {
        print("Failed to get " + url + ". HTTP code: " + std::to_string(res->status),
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    std::string cmakeOutput = m_Pd4WebRoot + asset;
    std::ofstream ofs(cmakeOutput, std::ios::binary);
    ofs << res->body;
    ofs.close();

    std::string destDir = m_Pd4WebRoot + "cmake";
    if (!fs::exists(destDir)) {
        fs::create_directories(destDir);
    }

    if (!extractTarGzToFolderStripTop(cmakeOutput, destDir)) {
        print("Failed to extract CMake binary", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    } 

    fs::remove(cmakeOutput);
    if (fs::exists(cmakeBinary)) {
        m_Cmake = cmakeBinary;
    } else {
        print("Empty cmake path", Pd4WebLogLevel::PD4WEB_ERROR);
        exit(-1);
    }

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
