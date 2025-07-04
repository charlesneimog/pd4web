#include "pd4web_compiler.hpp"

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
    print("Creating config.h file", Pd4WebLogLevel::LOG2, p->printLevel + 1);

    std::string configFile = readFile(p->Pd4WebFiles / "config.in.h");
    replaceAll(configFile, "@PD4WEB_VERSION_MAJOR@", "\"2\"");
    replaceAll(configFile, "@PD4WEB_VERSION_MINOR@", "\"4\"");
    replaceAll(configFile, "@PD4WEB_VERSION_PATCH@", "\"0.dev\"");

    replaceAll(configFile, "@PD4WEB_PROJECT_NAME@", "\"" + p->ProjectName + "\"");

    replaceAll(configFile, "@PD4WEB_CHS_IN@", std::to_string(p->Input));
    replaceAll(configFile, "@PD4WEB_CHS_OUT@", std::to_string(p->Output < 1 ? 1 : p->Output));
    replaceAll(configFile, "@PD4WEB_SR@", std::to_string(p->Sr));

    replaceAll(configFile, "@PD4WEB_GUI@", "true");
    replaceAll(configFile, "@PD4WEB_PATCH_ZOOM@", std::to_string(p->Zoom));
    replaceAll(configFile, "@PD4WEB_FPS@", "60");
    replaceAll(configFile, "@PD4WEB_AUTO_THEME@", "1");

    replaceAll(configFile, "@PD4WEB_MIDI@", std::to_string(p->Midi));

    writeFile((p->WebPatchFolder / "Pd4Web" / "config.h").string(), configFile);
}

// ─────────────────────────────────────
void Pd4Web::copySources(std::shared_ptr<Patch> &p) {
    fs::copy(p->Pd4WebFiles / "Libraries" / "libpd.cmake",
             p->WebPatchFolder / "Pd4Web" / "libpd.cmake", fs::copy_options::skip_existing);

    fs::create_directory(p->WebPatchFolder / "Pd4Web" / "pure-data");

    print("Copying pure-data sources to Pd4Web", Pd4WebLogLevel::LOG2, 2);
    fs::copy(p->Pd4WebRoot / "pure-data" / "src",
             p->WebPatchFolder / "Pd4Web" / "pure-data" / "src",
             fs::copy_options::recursive | fs::copy_options::skip_existing |
                 fs::copy_options::skip_symlinks);

    fs::copy(p->Pd4WebFiles / "pd4web.cpp", p->WebPatchFolder / "Pd4Web" / "pd4web.cpp",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.hpp", p->WebPatchFolder / "Pd4Web" / "pd4web.hpp",
             fs::copy_options::skip_existing);

    // Pd Lua
    if (p->PdLua) {
        print("Replacig lua sources from pdlua to pd4web", Pd4WebLogLevel::LOG2, 2);
        fs::copy(p->Pd4WebFiles / "pdlua.h",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua.h",
                 fs::copy_options::overwrite_existing);
        fs::copy(p->Pd4WebFiles / "pd4weblua_gfx.c",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua_gfx.h",
                 fs::copy_options::overwrite_existing);
        fs::copy(p->Pd4WebFiles / "Font" / "DejaVuSans.ttf",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "DejaVuSans.ttf",
                 fs::copy_options::skip_existing);
    }

    writeFile((p->WebPatchFolder / "index.html").string(),
              "<!doctype html><meta http-equiv=\"refresh\" content=\"0;url=WebPatch\">");

    // JavaScript
    fs::copy(p->Pd4WebFiles / "index.html", p->WebPatchFolder / "WebPatch" / "index.html",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "favicon.ico", p->WebPatchFolder / "favicon.ico",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.gui.js", p->WebPatchFolder / "WebPatch" / "pd4web.gui.js",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.threads.js",
             p->WebPatchFolder / "WebPatch" / "pd4web.threads.js", fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.style.css",
             p->WebPatchFolder / "WebPatch" / "pd4web.style.css", fs::copy_options::skip_existing);

    if (p->Midi) {
        fs::copy(p->Pd4WebFiles / "pd4web.midi.js",
                 p->WebPatchFolder / "WebPatch" / "pd4web.midi.js",
                 fs::copy_options::skip_existing);
    }
}

// ──────────────────────────────────────────
void Pd4Web::copyCmakeLibFiles(std::shared_ptr<Patch> &p, std::string LibName) {
    for (Library Lib : m_Libraries) {
        fs::path cmakeFile = p->WebPatchFolder / "Pd4Web" / "Externals" / (LibName + ".cmake");
        if (Lib.Name == LibName && !fs::exists(cmakeFile)) {
            print("Copying files of '" + LibName + "' to Pd4Web/Externals", Pd4WebLogLevel::LOG2,
                  2);
            fs::copy(p->Pd4WebRoot / LibName, p->WebPatchFolder / "Pd4Web" / "Externals" / LibName,
                     fs::copy_options::recursive | fs::copy_options::skip_existing |
                         fs::copy_options::skip_symlinks);

            print("Copying build script '" + LibName + ".cmake' to Pd4Web/Externals",
                  Pd4WebLogLevel::LOG2, 2);

            fs::copy(p->Pd4WebFiles / "Libraries" / (LibName + ".cmake"),
                     p->WebPatchFolder / "Pd4Web" / "Externals" / (LibName + ".cmake"),
                     fs::copy_options::skip_existing);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::createMainCmake(std::shared_ptr<Patch> &p) {
    print("Configuring CMakeLists.txt", Pd4WebLogLevel::LOG2, p->printLevel + 1);

    fs::create_directory(p->WebPatchFolder / "Pd4Web");
    fs::create_directory(p->WebPatchFolder / "Pd4Web" / "Externals");
    fs::create_directory(p->WebPatchFolder / "WebPatch");

    for (auto Lib : p->DeclaredLibs) {
        copyCmakeLibFiles(p, Lib);
    }
    for (auto Lib : p->DeclaredPaths) {
        copyCmakeLibFiles(p, Lib);
    }

    std::string cmakeTemplate = readFile(p->Pd4WebFiles / "Libraries" / "pd4web.in.cmake");

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

    std::vector<std::string> AddedLibs;

    for (auto Lib : p->DeclaredLibs) {
        if (Lib != "pdlua" &&
            std::find(AddedLibs.begin(), AddedLibs.end(), Lib) == AddedLibs.end()) {
            AddedLibs.push_back(Lib);
            LibrariesInclude +=
                "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/" + Lib + ".cmake\")\n";
            print("Including '" + Lib + "'", Pd4WebLogLevel::LOG2, p->printLevel + 2);
        }
    }

    for (std::string &Lib : p->DeclaredPaths) {
        for (Library supportedLib : m_Libraries) {
            if (supportedLib.Name == Lib &&
                std::find(AddedLibs.begin(), AddedLibs.end(), Lib) == AddedLibs.end()) {
                AddedLibs.push_back(Lib);
                LibrariesInclude +=
                    "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/" + Lib + ".cmake\")\n";
                print("Including '" + Lib + "'", Pd4WebLogLevel::LOG2, p->printLevel + 1);
            }
        }
    }

    if (p->PdLua) {
        LibrariesInclude +=
            "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua.cmake\")\n";
    }
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
    std::string baseDir = p->WebPatchFolder.string();
    std::vector<std::pair<std::string, std::string>> preloadMap = {
        {".tmp", "/Libs/"},
        {"Audios", "/Audios/"},
        {"Extras", "/Extras/"},
        {"WebPatch/index.pd", "index.pd"}};

    std::string preloadFlags;
    preloadFlags += " --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd";
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
    writeFile(p->WebPatchFolder.string() + "/CMakeLists.txt", cmakeTemplate);
}

// ─────────────────────────────────────
void Pd4Web::createExternalsCppFile(std::shared_ptr<Patch> &p) {
    print("Creating externals.cpp file", Pd4WebLogLevel::LOG2, p->printLevel + 1);
    print("\n");
    std::string externalsTemplate = readFile(p->Pd4WebFiles / "externals.in.cpp");

    std::string Declaration = "";
    std::string Call = "";
    if (p->PdLua) {
        Declaration += "extern \"C\" void pdlua_setup(void);\n";
        Call += "    pdlua_setup();\n";
    }

    std::string extraDefinitions = "";
    for (std::string Lib : p->DeclaredLibs) {
        if (Lib == "cyclone") {
            extraDefinitions += "#if !defined(CYCLONE_OBJ_API)\n#define CYCLONE_OBJ_API\n#endif\n";
        }
    }

    for (PatchLine &pl : p->ExternalPatchLines) {
        if (pl.isAbstraction && pl.Type != PatchLine::OBJ && pl.OriginalTokens[1] != "obj") {
            continue;
        }

        if (pl.isExternal && !pl.isLuaExternal && !pl.Lib.empty()) {
            if (p->ExternalObjectsJson.contains(pl.Lib)) {
                if (p->ExternalObjectsJson[pl.Lib].contains("objects")) {
                    if (p->ExternalObjectsJson[pl.Lib]["objects"].contains(pl.Name)) {
                        print("Found " +
                                  p->ExternalObjectsJson[pl.Lib]["objects"][pl.Name][0]
                                      .get<std::string>() +
                                  " setup function",
                              Pd4WebLogLevel::LOG2, p->printLevel + 1);
                        Declaration += "extern \"C\" " +
                                       p->ExternalObjectsJson[pl.Lib]["objects"][pl.Name][0]
                                           .get<std::string>() +
                                       ";\n";
                        Call += "    " +
                                p->ExternalObjectsJson[pl.Lib]["objects"][pl.Name][1]
                                    .get<std::string>() +
                                "();\n";
                    } else {
                        print("No objects name key " + pl.Name, Pd4WebLogLevel::ERROR,
                              p->printLevel + 1);
                    }
                } else {
                    print("Something went wrong (no objects key) " + pl.Name, Pd4WebLogLevel::ERROR,
                          p->printLevel + 1);
                }
            } else {
                print("Something went wrong (no objects lib) " + pl.Name, Pd4WebLogLevel::ERROR,
                      p->printLevel + 1);
            }
        } else if (pl.isExternal && !pl.isLuaExternal && pl.Lib.empty() && !pl.Name.empty()) {
            if (p->ExternalObjectsJson.contains(pl.Name)) {
                if (p->ExternalObjectsJson[pl.Name]["objects"].contains(pl.Name)) {
                    print("Found " +
                              p->ExternalObjectsJson[pl.Name]["objects"][pl.Name][0]
                                  .get<std::string>() +
                              " setup function",
                          Pd4WebLogLevel::LOG2, p->printLevel + 1);
                    Declaration +=
                        "extern \"C\" " +
                        p->ExternalObjectsJson[pl.Name]["objects"][pl.Name][0].get<std::string>() +
                        ";\n";
                    Call +=
                        "    " +
                        p->ExternalObjectsJson[pl.Name]["objects"][pl.Name][1].get<std::string>() +
                        "();\n";
                } else {
                    print("Something went wrong no name " + pl.Name, Pd4WebLogLevel::ERROR,
                          p->printLevel + 1);
                }
            } else {
                print("Something went wrong no objects " + pl.Name, Pd4WebLogLevel::ERROR,
                      p->printLevel + 1);
            }
        } else {
            print("Something went wrong for no lib" + pl.Name, Pd4WebLogLevel::ERROR,
                  p->printLevel + 1);
        }
    }

    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_EXTRA@", extraDefinitions);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_DECLARATION@", Declaration);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_SETUP@", Call);
    writeFile(p->WebPatchFolder.string() + "/Pd4Web/externals.cpp", externalsTemplate);
}

// ─────────────────────────────────────
void Pd4Web::configureProjectToCompile(std::shared_ptr<Patch> &p) {

    print("\n");
    print("Configuring Build Project", Pd4WebLogLevel::LOG1);

    createMainCmake(p);
    copySources(p);

    print("\n");
    print("Configuring C++ Code", Pd4WebLogLevel::LOG1);
    createConfigFile(p);
    createExternalsCppFile(p);
}

// ─────────────────────────────────────
void Pd4Web::buildPatch(std::shared_ptr<Patch> &p) {
    std::string releaseType = "Release"; // or get from config
    if (m_Debug) {
        releaseType = "Debug";
    }
    std::string configure = m_Emconfigure;
    std::string make = m_Emmake;
    std::string fullCommand;

    fs::create_directory(p->WebPatchFolder / ".build");

#ifdef _WIN32
    auto path = (p->WebPatchFolder / ".build").string();
    DWORD attrs = GetFileAttributes(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        SetFileAttributes(path.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
    }
#endif

    // TODO: on windows I need to set this cmake as binary
    std::vector<std::string> command = {m_Emcmake,
                                        m_Cmake,
                                        p->WebPatchFolder.string(),
                                        "-B",
                                        (p->WebPatchFolder / ".build").string(),
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
    std::vector<std::string> buildCmd = {m_Cmake,
                                         "--build",
                                         (p->WebPatchFolder / ".build").string(),
                                         "-j" + std::to_string(cpuCount),
                                         "--target",
                                         "pd4web"};

    std::string buildStr;
    for (const auto &arg : buildCmd) {
        buildStr += arg + " ";
    }

    ret = std::system(buildStr.c_str());
    if (ret != 0) {
        std::cerr << "Build failed\n";
    }
}
