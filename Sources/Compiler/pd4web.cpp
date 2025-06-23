#include "pd4web.hpp"

Pd4Web::Pd4Web(std::string pathHome) {
    LOG(__PRETTY_FUNCTION__);

    print("\n", Pd4WebColor::GREEN);
    m_Pd4WebRoot = pathHome;
    if (pathHome.back() != '/') {
        m_Pd4WebRoot += "/";
    }
    git_libgit2_init();
    m_cppParser = ts_parser_new();
    m_cParser = ts_parser_new();

    ts_parser_set_language(m_cppParser, tree_sitter_cpp());
    ts_parser_set_language(m_cParser, tree_sitter_c());

    // clone emscripten and pd
    bool ok = gitClone("https://github.com/emscripten-core/emsdk.git", "emsdk", EMSDK_VERSION);
    if (!ok) {
        return;
    }
    ok = gitClone("https://github.com/pure-data/pure-data.git", "pure-data", PD_VERSION);
    if (!ok) {
        return;
    }

    // install emscripten
    ok = initPaths();
    if (!ok) {
        std::cout << "Failed to init paths" << std::endl;
        return;
    }

    ok = checkAllPaths();
    if (!ok) {
        ok = cmdInstallEmsdk();
        if (!ok) {
            return;
        }
        ok = checkAllPaths();
        if (!ok) {
            return;
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::parseArgs(int argc, char *argv[]) {
    LOG(__PRETTY_FUNCTION__);
    cxxopts::Options options("pd4web", "Compile Pure Data externals for the web");

    options.add_options()("v,verbose", "Enable verbose")(
        "m,initial-memory", "Initial memory size (in MB)", cxxopts::value<int>(m_Memory))(
        "nogui", "Disable GUI", cxxopts::value<bool>(m_RenderGui)->default_value("true"))(
        "z,patch-zoom", "Patch zoom", cxxopts::value<float>(m_PatchZoom)->default_value("1"))(
        "patch_file", "Patch file to be compiled", cxxopts::value<std::string>(m_PatchFile))(
        "template", "Template ID",
        cxxopts::value<int>(m_TemplateId))("debug", "Debug mode", cxxopts::value<bool>(m_Debug));

    options.parse_positional({"patch_file"});
    auto result = options.parse(argc, argv);
    if (result.count("patch_file")) {
        std::string patchFile = result["patch_file"].as<std::string>();
        LOG(patchFile);
    }
}
