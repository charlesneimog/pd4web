#include "pd4web.hpp"

#include <string>
#include <thread>

// ─────────────────────────────────────
std::string pdCMakeBlock = "# Pd Cmake\n"
                           "set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)\n"
                           "set(PDCMAKE_VERSION \"v0.2.0\")\n"
                           "if(NOT EXISTS \"${PDCMAKE_FILE}\")\n"
                           "    file(DOWNLOAD "
                           "https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/"
                           "${PDCMAKE_VERSION}/pd.cmake ${PDCMAKE_FILE})\n"
                           "endif()\n"
                           "include(${PDCMAKE_FILE})";

// ─────────────────────────────────────
void Pd4Web::createConfigFile(std::shared_ptr<Patch> &p) {
    print("Create config.h file", Pd4WebColor::GREEN, p->printLevel + 1);

    std::string configFile = readFile("/home/neimog/Documents/Git/pd4web/Sources/config.in.h");
    replaceAll(configFile, "@PD4WEB_VERSION_MAJOR@", "\"2\"");
    replaceAll(configFile, "@PD4WEB_VERSION_MINOR@", "\"4\"");
    replaceAll(configFile, "@PD4WEB_VERSION_PATCH@", "\"0.dev\"");

    replaceAll(configFile, "@PD4WEB_PROJECT_NAME@", "\"" + p->ProjectName + "\"");

    replaceAll(configFile, "@PD4WEB_CHS_IN@", std::to_string(p->Input));
    replaceAll(configFile, "@PD4WEB_CHS_OUT@", std::to_string(p->Output));
    replaceAll(configFile, "@PD4WEB_SR@", std::to_string(p->Sr));

    replaceAll(configFile, "@PD4WEB_GUI@", "true");
    replaceAll(configFile, "@PD4WEB_PATCH_ZOOM@", std::to_string(p->Zoom));
    replaceAll(configFile, "@PD4WEB_FPS@", "60");
    replaceAll(configFile, "@PD4WEB_AUTO_THEME@", "1");

    replaceAll(configFile, "@PD4WEB_MIDI@", std::to_string(p->Midi));

    writeFile((p->Root / "Pd4Web" / "config.h").string(), configFile);
}

// ─────────────────────────────────────
void Pd4Web::copySources(std::shared_ptr<Patch> &p) {
    fs::copy("/home/neimog/Documents/Git/pd4web/Sources/Libraries/libpd.cmake",
             p->Root / "Pd4Web" / "libpd.cmake", fs::copy_options::skip_existing);

    fs::create_directory(p->Root / "Pd4Web" / "pure-data");

    print("Copying pure-data sources to Pd4Web", Pd4WebColor::GREEN, 2);
    fs::copy(p->Pd4WebRoot / "pure-data" / "src", p->Root / "Pd4Web" / "pure-data" / "src",
             fs::copy_options::recursive | fs::copy_options::skip_existing |
                 fs::copy_options::skip_symlinks);

    fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4web.cpp",
             p->Root / "Pd4Web" / "pd4web.cpp", fs::copy_options::skip_existing);
    fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4web.hpp",
             p->Root / "Pd4Web" / "pd4web.hpp", fs::copy_options::skip_existing);

    // Pd Lua
    if (p->PdLua) {
        print("Replacig lua sources from pdlua to pd4web", Pd4WebColor::GREEN, 2);
        fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pdlua.h",
                 p->Root / "Pd4Web" / "Externals" / "pdlua" / "pdlua.h",
                 fs::copy_options::overwrite_existing);
        fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4weblua_gfx.c",
                 p->Root / "Pd4Web" / "Externals" / "pdlua" / "pdlua_gfx.h",
                 fs::copy_options::overwrite_existing);
        fs::copy("/home/neimog/Documents/Git/pd4web/Sources/Font/DejaVuSans.ttf",
                 p->Root / "Pd4Web" / "Externals" / "pdlua" / "DejaVuSans.ttf",
                 fs::copy_options::skip_existing);
    }

    // JavaScript
    fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4web.gui.js",
             p->Root / "WebPatch" / "pd4web.gui.js", fs::copy_options::skip_existing);
    fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4web.threads.js",
             p->Root / "WebPatch" / "pd4web.threads.js", fs::copy_options::skip_existing);
    fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4web.style.css",
             p->Root / "WebPatch" / "pd4web.style.css", fs::copy_options::skip_existing);
    if (p->Midi) {
        fs::copy("/home/neimog/Documents/Git/pd4web/Sources/pd4web.midi.js",
                 p->Root / "WebPatch" / "pd4web.midi.js", fs::copy_options::skip_existing);
    }
}

// ─────────────────────────────────────
void Pd4Web::createMainCmake(std::shared_ptr<Patch> &p) {
    print("Configuring CMakeLists.txt", Pd4WebColor::GREEN, p->printLevel + 1);
    print("\n");

    fs::create_directory(p->Root / "Pd4Web");
    fs::create_directory(p->Root / "Pd4Web" / "Externals");
    fs::create_directory(p->Root / "WebPatch");

    for (auto Lib : p->UsedLibs) {
        print("Copying files of '" + Lib + "' to Pd4Web/Externals", Pd4WebColor::GREEN, 2);
        fs::copy(p->Pd4WebRoot / Lib, p->Root / "Pd4Web" / "Externals" / Lib,
                 fs::copy_options::recursive | fs::copy_options::skip_existing |
                     fs::copy_options::skip_symlinks);

        // TODO:
        if (Lib == "pdlua") {
            // we replace the pdlua_gfx
        }

        print("Copying build script '" + Lib + ".cmake' to Pd4Web/Externals", Pd4WebColor::GREEN,
              2);

        // TODO: Replace with right file
        fs::copy("/home/neimog/Documents/Git/pd4web/Sources/Libraries/" + Lib + ".cmake",
                 p->Root / "Pd4Web" / "Externals" / (Lib + ".cmake"),
                 fs::copy_options::skip_existing);
        print("\n");
    }

    // TODO: Replace with right file
    std::string cmakeTemplate =
        readFile("/home/neimog/Documents/Git/pd4web/Sources/Libraries/pd4web.in.cmake");

    replaceAll(cmakeTemplate, "@PROJECT_NAME@", p->ProjectName);
    replaceAll(cmakeTemplate, "@MEMORY_SIZE@", std::to_string(p->MemorySize));

    // Lua (condicional)
    if (p->PdLua) {
        replaceAll(
            cmakeTemplate, "@PD4WEB_LUA_DEFS@",
            "add_definitions(-DPD4WEB_LUA)\n"
            "include_directories(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua/lua\")");
    } else {
        replaceAll(cmakeTemplate, "@PD4WEB_LUA_DEFS@", "");
    }

    // cmake external includes
    replaceAll(cmakeTemplate, "@PD_CMAKE_CONTENT@", pdCMakeBlock);
    std::string LibrariesInclude = "";
    for (auto Lib : p->UsedLibs) {
        // avoid conflict with else
        if (Lib != "pdlua") {
            LibrariesInclude +=
                "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/" + Lib + ".cmake\")\n";
        }
    }
    LibrariesInclude += "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua.cmake\")\n";
    replaceAll(cmakeTemplate, "@LIBRARIES_SCRIPT_INCLUDE@", LibrariesInclude);

    // externals objects targets

    std::string externalTargets = "";
    if (p->PdLua) {
        externalTargets += "\n" + std::string(32, ' ') + "pdlua";
    }

    for (auto &pl : p->ExternalPatchLines) {
        if (pl.isExternal) {
            std::string str = pl.Name;
            size_t pos = 0;
            while ((pos = str.find('~', pos)) != std::string::npos) {
                str.replace(pos, 1, "_tilde");
                pos += 6;
            }
            externalTargets += "\n" + std::string(32, ' ') + str;
        }
    }

    std::string ExternalsTargets = "target_link_libraries(pd4web PRIVATE " + externalTargets + ")";
    replaceAll(cmakeTemplate, "@PD4WEB_EXTERNAL_OBJECTS_TARGET@", ExternalsTargets);

    // Preload files
    std::string PreloadedFiles = "get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)\n"
                                 "if(NOT EMCC_LINK_FLAGS)\n"
                                 "    set(EMCC_LINK_FLAGS \"\")\n"
                                 "endif()\n";

    // Collect all --preload-file entries into one string
    std::string baseDir = p->mainRoot.string();
    std::vector<std::pair<std::string, std::string>> preloadMap = {
        {"WebPatch/index.pd", "/index.pd"},
        {".tmp", "/Libs/"},
        {"Audios", "/Audios/"},
        {"Extras", "/Extras/"},
        {"WebPatch/index.pd", "index.pd"}};
    std::string preloadFlags;
    for (const auto &[srcRel, dstRel] : preloadMap) {
        fs::path fullSrc = fs::path(baseDir) / srcRel;
        if (fs::exists(fullSrc)) {
            preloadFlags += " --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/" + srcRel + "@" + dstRel;
        }
    }

    // Generate the full CMake snippet
    PreloadedFiles += "set_target_properties(pd4web PROPERTIES LINK_FLAGS \"${EMCC_LINK_FLAGS}" +
                      preloadFlags + "\")\n";

    // Replace in template
    replaceAll(cmakeTemplate, "@PD4WEB_PRELOADED_PATCH@", PreloadedFiles);

    // Write cmake
    writeFile(p->Root.string() + "/CMakeLists.txt", cmakeTemplate);
}

// ─────────────────────────────────────
void Pd4Web::createExternalsCppFile(std::shared_ptr<Patch> &p) {
    // TODO: Replace to right file
    print("Create externals.cpp file", Pd4WebColor::GREEN, p->printLevel + 1);
    std::string externalsTemplate =
        readFile("/home/neimog/Documents/Git/pd4web/Sources/externals.in.cpp");

    std::string Declaration = "";
    std::string Call = "";
    if (p->PdLua) {
        Declaration += "extern \"C\" void pdlua_setup(void);\n";
        Call += "    pdlua_setup();\n";
    }

    for (PatchLine &pl : p->ExternalPatchLines) {
        if (pl.isExternal && !pl.isLuaExternal && !pl.Lib.empty()) {
            if (p->ExternalObjectsJson.contains(pl.Lib)) {
                if (p->ExternalObjectsJson[pl.Lib].contains(pl.Name)) {
                    Declaration += "extern \"C\" " +
                                   p->ExternalObjectsJson[pl.Lib][pl.Name][0].get<std::string>() +
                                   ";\n";
                    Call += "    " + p->ExternalObjectsJson[pl.Lib][pl.Name][1].get<std::string>() +
                            "();\n";
                }
            }
        } else if (pl.isExternal && !pl.isLuaExternal && pl.Lib.empty()) {
            if (p->ExternalObjectsJson.contains(pl.Name)) {
                if (p->ExternalObjectsJson[pl.Name].contains(pl.Name)) {
                    Declaration += "extern \"C\" " +
                                   p->ExternalObjectsJson[pl.Name][pl.Name][0].get<std::string>() +
                                   ";\n";
                    Call += "    " +
                            p->ExternalObjectsJson[pl.Name][pl.Name][1].get<std::string>() +
                            "();\n";
                }
            }
        }
    }

    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_DECLARATION@", Declaration);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_SETUP@", Call);
    writeFile(p->Root.string() + "/Pd4Web/externals.cpp", externalsTemplate);
}

// ─────────────────────────────────────
void Pd4Web::configureProjectToCompile(std::shared_ptr<Patch> &p) {

    print("\n");
    print("Configuring Build Project", Pd4WebColor::BLUE);

    createMainCmake(p);
    copySources(p);

    print("\n");
    print("Configuring C++ Code", Pd4WebColor::BLUE);
    createConfigFile(p);
    createExternalsCppFile(p);
}

// ─────────────────────────────────────
void Pd4Web::buildPatch(std::shared_ptr<Patch> &p) {
    std::string releaseType = "Release"; // or get from config
    std::string configure = m_Emconfigure;
    std::string make = m_Emmake;
    std::string fullCommand;
    std::vector<std::string> command = {m_Emcmake, // usually "emcmake"
                                        "cmake",
                                        p->mainRoot.string(),
                                        "-B",
                                        (p->mainRoot / "build").string(),
                                        "-G",
                                        "Ninja",
                                        "-DCMAKE_MAKE_PROGRAM=" +
                                            m_Ninja, // full path to Ninja from emsdk
                                        "-DPDCMAKE_DIR=Pd4Web/Externals/",
                                        "-DCMAKE_BUILD_TYPE=" + releaseType,
                                        "-DEMCONFIGURE=" + configure,
                                        "-DEMMAKE=" + make,
                                        "-Wno-dev"};

    for (const auto &arg : command) {
        fullCommand += arg + " ";
    }
    int ret = std::system(fullCommand.c_str());

    // build
    int cpuCount = std::thread::hardware_concurrency();
    std::vector<std::string> buildCmd = {
        "cmake",    "--build", (p->mainRoot / "build").string(), "-j" + std::to_string(cpuCount),
        "--target", "pd4web"};

    std::string buildStr;
    for (const auto &arg : buildCmd) {
        buildStr += arg + " ";
    }

    ret = std::system(buildStr.c_str());
    if (ret != 0) {
        std::cerr << "Build failed\n";
    }
}
