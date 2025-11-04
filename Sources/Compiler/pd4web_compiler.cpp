#include "pd4web_compiler.hpp"

Pd4Web::Pd4Web(std::string pathHome) {
    m_Init = false;
    m_Error = false;
    m_Pd4WebRoot = pathHome;
    m_Pd4WebFiles = fs::path(m_Pd4WebRoot) / "Pd4Web";
}

// ─────────────────────────────────────
bool Pd4Web::init() {
    if (m_Pd4WebRoot.back() != '/') {
        m_Pd4WebRoot += "/";
    }
    print("Initializing pd4web", Pd4WebLogLevel::PD4WEB_LOG1);

    // libgit2
    git_libgit2_init();

#if defined(_WIN32)
    char pythonPath[MAX_PATH];
    DWORD pyLen = SearchPathA(nullptr, "python.exe", nullptr, MAX_PATH, pythonPath, nullptr);

    // Inline check for Python stub (Windows Store) or missing
    bool needInstall = false;
    if (pyLen == 0) {
        needInstall = true;
    } else {
        HANDLE hFile = CreateFileA(pythonPath, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, 0, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            needInstall = true;
        } else {
            DWORD fileSize = GetFileSize(hFile, nullptr);
            CloseHandle(hFile);
            if (fileSize < 5000) {
                needInstall = true;
            }
        }
    }

    if (needInstall) {
        print("Python not found or is a Windows Store stub. Attempting to install via winget...",
              Pd4WebLogLevel::PD4WEB_LOG2);

        char wingetPath[MAX_PATH];
        DWORD len = SearchPathA(nullptr, "winget.exe", nullptr, MAX_PATH, wingetPath, nullptr);
        if (len == 0) {
            print("winget not found. Please install Python manually! Go to https://www.python.org/downloads/",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        std::string wingetExe = (len > 0) ? std::string(wingetPath) : "winget";
        std::vector<std::string> wingetCheckCmd = {"install", "-e", "--id=Python.Python.3.11", "--accept-source-agreements", "--accept-package-agreements"};
        int wingetResult = execProcess(wingetExe, wingetCheckCmd);
        if (wingetResult != 0) {
            print("Failed to install Python via winget. Please install Python manually! Go to https://www.python.org/downloads/",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        print("Python installed successfully via winget.", Pd4WebLogLevel::PD4WEB_LOG2);
    } else {
        print("Python installation appears valid.", Pd4WebLogLevel::PD4WEB_LOG2);
    }

    pyLen = SearchPathA(nullptr, "python.exe", nullptr, MAX_PATH, pythonPath, nullptr);
    if (pyLen == 0) {
        print("Python interpreter not found after installation attempt. Please install Python manually. Go to https://www.python.org/downloads/",
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    m_PythonWindows = pythonPath;
    _putenv_s("EMSDK_PY", pythonPath);
    print("Using Python interpreter at: " + std::string(pythonPath), Pd4WebLogLevel::PD4WEB_LOG2);
    print("SSL certificate: " + getCertFile(), Pd4WebLogLevel::PD4WEB_LOG2);
#endif

    // libtree-sitter
    m_cppParser = ts_parser_new();
    m_cParser = ts_parser_new();
    ts_parser_set_language(m_cppParser, tree_sitter_cpp());
    ts_parser_set_language(m_cParser, tree_sitter_c());

    // clone emscripten and pd
    print("Checking emsdk", Pd4WebLogLevel::PD4WEB_LOG2);
    bool ok = gitClone("https://github.com/emscripten-core/emsdk.git", "emsdk", EMSDK_VERSION);
    if (!ok) {
        return false;
    }

    // clone pure-data
    print("Checking pure-data", Pd4WebLogLevel::PD4WEB_LOG2);
    ok = gitClone("https://github.com/pure-data/pure-data.git", "pure-data", PUREDATA_VERSION);
    if (!ok) {
        return false;
    }

    // clone pd-lua
    ok = gitClone("https://github.com/agraef/pd-lua.git", "pdlua", "0.12.23");
    if (!ok) {
        print("Failed to clone pd-lua", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // clone pd.cmake
    ok = gitClone("https://github.com/pure-data/pd.cmake.git", "pd.cmake", "v0.2.6");
    if (!ok) {
        print("Failed to clone pd.cmake", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // install emscripten
    print("Initializing paths", Pd4WebLogLevel::PD4WEB_LOG2);
    ok = initPaths();
    if (!ok) {
        std::cout << "Failed to init paths" << std::endl;
        return false;
    }

    // install emscripten
    ok = cmdInstallEmsdk();
    if (!ok) {
        return false;
    }

    // check all paths
    ok = checkAllPaths();
    if (!ok) {
        return false;
    }

    m_Init = true;
    m_Error = false;
    print("Pd4Web initialized successfully", Pd4WebLogLevel::PD4WEB_LOG1);

    return true;
}

// ─────────────────────────────────────
void Pd4Web::parseArgs(int argc, char *argv[]) {
    cxxopts::Options options("pd4web", "pd4web compiles PureData patches with external objects for "
                                       "Wasm, allowing to run entire patches in web browsers.\n");

    bool disableGui = false;
    bool failFast = false;

    // clang-format off
    options.add_options()
        // help
        ("help", "Print this usage.")

        // Configuration of Compiler
        ("pd4web-folder", "Pd4Web Folder (with Libraries, Sources, etc).",
            cxxopts::value<fs::path>(m_Pd4WebFiles))

        // Configuration of Pd4Web
        ("m,initial-memory", "Initial memory size (in MB).", 
            cxxopts::value<int>(m_Memory)->default_value("32"))

        ("z,patch-zoom", "Patch zoom.", 
            cxxopts::value<float>(m_PatchZoom)->default_value("1"))

        ("o,output-folder", "Output folder.", 
            cxxopts::value<std::string>(m_OutputFolder))

        ("patch_file", "Patch file to be compiled.", 
            cxxopts::value<std::string>(m_PatchFile))

        // TODO:
        ("t,template-id", "Set template id check https://charlesneimog.github.io/pd4web/patch/templates/.",
            cxxopts::value<int>(m_TemplateId))


        // TEST:
        ("nogui", "Disable GUI interface.", 
            cxxopts::value<bool>(disableGui))

        ("debug", "Activate debug compilation (faster compilation, slower and more error info execution).", 
            cxxopts::value<bool>(m_Debug))

        ("devdebug", "Activate development debug compilation (print function call).", 
            cxxopts::value<bool>(m_DevDebug)->default_value("false"))

        ("failfast", "Fail on first error message.",
            cxxopts::value<bool>(failFast));

    // clang-format on
    options.parse_positional({"patch_file"});
    auto result = options.parse(argc, argv);

    m_RenderGui = !disableGui;
    m_FailFast = failFast;

    if (result.count("help")) {
        options.set_width(120);
        std::cout << options.help() << std::endl;
        exit(0);
    }
}
