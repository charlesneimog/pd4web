#include "pd4web.hpp"

Pd4Web::Pd4Web(std::string pathHome) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
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

    print("Initializing pd4web", Pd4WebLogLevel::LOG1);
    git_libgit2_init();
    m_cppParser = ts_parser_new();
    m_cParser = ts_parser_new();

    ts_parser_set_language(m_cppParser, tree_sitter_cpp());
    ts_parser_set_language(m_cParser, tree_sitter_c());

    // clone emscripten and pd
    print("Checking emsdk", Pd4WebLogLevel::LOG2);
    bool ok = gitClone("https://github.com/emscripten-core/emsdk.git", "emsdk", EMSDK_VERSION);
    if (!ok) {
        return false;
    }

    print("Checking pure-data", Pd4WebLogLevel::LOG2);
    ok = gitClone("https://github.com/pure-data/pure-data.git", "pure-data", PD_VERSION);
    if (!ok) {
        return false;
    }

    // install emscripten
    print("Initializing paths", Pd4WebLogLevel::LOG2);
    ok = initPaths();
    if (!ok) {
        std::cout << "Failed to init paths" << std::endl;
        return false;
    }

    ok = checkAllPaths();
    if (!ok) {
        ok = cmdInstallEmsdk();
        if (!ok) {
            return false;
        }
        ok = checkAllPaths();
        if (!ok) {
            return false;
        }
    }
    m_Init = true;

    return true;
}

// ─────────────────────────────────────
void Pd4Web::parseArgs(int argc, char *argv[]) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    cxxopts::Options options("pd4web", "pd4web compiles PureData patches with external objects for "
                                       "Wasm, allowing to run entire patches in web browsers.\n");

    bool disableGui = false;

    // clang-format off
    options.add_options()
        ("h,help", "Print this usage.")

        // // TODO:
        // ("v,verbose", "Enable verbose.",
        //     cxxopts::value<bool>(m_Verbose)->default_value("false"))

        // TODO:
        ("m,initial-memory", "Initial memory size (in MB).", 
            cxxopts::value<int>(m_Memory)->default_value("32"))


        // TODO:
        ("z,patch-zoom", "Patch zoom.", 
            cxxopts::value<float>(m_PatchZoom)->default_value("1"))

        // TODO:
        ("o,output-folder", "Output folder.", 
            cxxopts::value<std::string>(m_OutputFolder))

        ("patch_file", "Patch file to be compiled.", 
            cxxopts::value<std::string>(m_PatchFile))

        // TODO:
        ("t,template-id", "Set template id check https://charlesneimog.github.io/pd4web/patch/templates/.",
            cxxopts::value<int>(m_TemplateId))


        // TODO:
        // ("server", "Run pd4web server",
        //     cxxopts::value<int>(m_TemplateId))
        ("nogui", "Disable GUI interface.", 
            cxxopts::value<bool>(disableGui))

        ("debug", "Activate debug compilation (faster compilation, slower and more error info execution).", 
            cxxopts::value<bool>(m_Debug))

        ("devdebug", "Activate development debug compilation (print function call).", 
            cxxopts::value<bool>(m_DevDebug)->default_value("false"));

    // clang-format on
    options.parse_positional({"patch_file"});
    auto result = options.parse(argc, argv);

    m_RenderGui = !disableGui;

    if (result.count("patch_file")) {
        std::string patchFile = result["patch_file"].as<std::string>();
        print(patchFile, Pd4WebLogLevel::VERBOSE);
    }

    if (result.count("help")) {
        options.set_width(120);
        std::cout << options.help() << std::endl;
        exit(0);
    }
}
