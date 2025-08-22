#include "pd4web_compiler.hpp"

#include <string>
#include <thread>
#include <fstream>

#define BOOST_PROCESS_VERSION 2
#include <boost/process.hpp>
#include <boost/asio.hpp>

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
    PD4WEB_LOGGER();
    print("Creating config.h file", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    std::string configFile = readFile((p->Pd4WebFiles / "config.in.h").string());
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
    if (p->PdLua) {
        replaceAll(configFile, "@PD4WEB_LUA@", "1");
    } else {
        replaceAll(configFile, "@PD4WEB_LUA@", "0");
    }

    replaceAll(configFile, "@PD4WEB_MIDI@", std::to_string(p->Midi));

    writeFile((p->WebPatchFolder / "Pd4Web" / "config.h").string(), configFile);
}

// ─────────────────────────────────────
void Pd4Web::copySources(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();
    fs::copy(p->Pd4WebFiles / "Libraries" / "libpd.cmake",
             p->WebPatchFolder / "Pd4Web" / "libpd.cmake", fs::copy_options::skip_existing);

    fs::create_directory(p->WebPatchFolder / "Pd4Web" / "pure-data");

    print("Copying pure-data sources to Pd4Web", Pd4WebLogLevel::PD4WEB_LOG2, 2);
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
        if (!fs::exists(p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua")) {
            fs::copy(p->Pd4WebRoot / "pdlua", p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua",
                     fs::copy_options::recursive);
        }

        print("Replacig lua sources from pdlua to pd4web", Pd4WebLogLevel::PD4WEB_LOG2, 2);

        fs::copy(p->Pd4WebFiles / "pd4weblua.c",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua.c",
                 fs::copy_options::overwrite_existing);
        fs::copy(p->Pd4WebFiles / "pd4weblua.h",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua.h",
                 fs::copy_options::overwrite_existing);
        fs::copy(p->Pd4WebFiles / "pd4weblua_gfx.c",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua_gfx.h",
                 fs::copy_options::overwrite_existing);

        fs::copy(p->Pd4WebFiles / "DejaVuSans.ttf",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua" / "DejaVuSans.ttf",
                 fs::copy_options::skip_existing);
    }

    writeFile((p->WebPatchFolder / "index.html").string(),
              "<!doctype html><meta http-equiv=\"refresh\" content=\"0;url=WebPatch\">");

    // Lua Gui Objects
    if (p->LuaGuiObjects) {
        fs::copy(p->Pd4WebFiles / "Gui", p->WebPatchFolder / "Pd4Web" / "Gui",
                 fs::copy_options::recursive | fs::copy_options::skip_existing);
    }

    // JavaScript
    fs::copy(p->Pd4WebFiles / "index.html", p->WebPatchFolder / "WebPatch" / "index.html",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "favicon.ico", p->WebPatchFolder / "favicon.ico",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.sw.js", p->WebPatchFolder / "WebPatch" / "pd4web.sw.js",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "icon-512.png", p->WebPatchFolder / "WebPatch" / "icon-512.png",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "icon-192.png", p->WebPatchFolder / "WebPatch" / "icon-192.png",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.threads.js",
             p->WebPatchFolder / "WebPatch" / "pd4web.threads.js", fs::copy_options::skip_existing);
}

// ──────────────────────────────────────────
void Pd4Web::copyCmakeLibFiles(std::shared_ptr<Patch> &p, std::string LibName) {
    PD4WEB_LOGGER();
    for (Library Lib : m_Libraries) {
        fs::path cmakeFile = p->WebPatchFolder / "Pd4Web" / "Externals" / (LibName + ".cmake");
        if (Lib.Name == LibName && !fs::exists(cmakeFile)) {
            print("Copying files of '" + LibName + "' to Pd4Web/Externals",
                  Pd4WebLogLevel::PD4WEB_LOG2, 2);
            fs::copy(p->Pd4WebRoot / LibName, p->WebPatchFolder / "Pd4Web" / "Externals" / LibName,
                     fs::copy_options::recursive | fs::copy_options::skip_existing |
                         fs::copy_options::skip_symlinks);

            print("Copying build script '" + LibName + ".cmake' to Pd4Web/Externals",
                  Pd4WebLogLevel::PD4WEB_LOG2, 2);

            fs::copy(p->Pd4WebFiles / "Libraries" / (LibName + ".cmake"),
                     p->WebPatchFolder / "Pd4Web" / "Externals" / (LibName + ".cmake"),
                     fs::copy_options::skip_existing);
        }
    }

    if (p->PdLua) {
        fs::copy(p->Pd4WebFiles / "Libraries" / "pdlua.cmake",
                 p->WebPatchFolder / "Pd4Web" / "Externals" / "pdlua.cmake",
                 fs::copy_options::skip_existing);
    }
}

// ─────────────────────────────────────
void Pd4Web::createMainCmake(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    //
    print("Configuring CMakeLists.txt", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    fs::create_directory(p->WebPatchFolder / "Pd4Web");
    fs::create_directory(p->WebPatchFolder / "Pd4Web" / "Externals");
    fs::create_directory(p->WebPatchFolder / "WebPatch");

    for (auto Lib : p->DeclaredLibs) {
        copyCmakeLibFiles(p, Lib);
    }
    for (auto Lib : p->DeclaredPaths) {
        copyCmakeLibFiles(p, Lib);
    }

    if (p->PdLua) {
        copyCmakeLibFiles(p, "pdlua");
    }

    std::string cmakeTemplate =
        readFile((p->Pd4WebFiles / "Libraries" / "pd4web.in.cmake").string());

    replaceAll(cmakeTemplate, "@PROJECT_NAME@", p->ProjectName);
    replaceAll(cmakeTemplate, "@MEMORY_SIZE@", std::to_string(p->MemorySize));
    // TODO: Version
    replaceAll(cmakeTemplate, "@LIBPD_TAG@", "0.56-0");

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
            print("Including '" + Lib + "'", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 2);
        }
    }

    for (std::string &Lib : p->DeclaredPaths) {
        for (Library supportedLib : m_Libraries) {
            if (supportedLib.Name == Lib &&
                std::find(AddedLibs.begin(), AddedLibs.end(), Lib) == AddedLibs.end()) {
                AddedLibs.push_back(Lib);
                LibrariesInclude +=
                    "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/" + Lib + ".cmake\")\n";
                print("Including '" + Lib + "'", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
            }
        }
    }

    // pdlua is separeted to avoid conflict with else
    LibrariesInclude += "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua.cmake\")\n";
    replaceAll(cmakeTemplate, "@LIBRARIES_SCRIPT_INCLUDE@", LibrariesInclude);

    std::string pdsource =
        "set(PD_SOURCE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/\")";
    replaceAll(cmakeTemplate, "@PD_SOURCE_DIR@", pdsource);

    // extra definitions
    if (m_Debug && m_DevDebug) {
        replaceAll(cmakeTemplate, "@PD_CMAKE_EXTRADEFINITIONS@", "");
    } else {
        std::string extra = "add_compile_options(-Wno-everything)\n";
        replaceAll(cmakeTemplate, "@PD_CMAKE_EXTRADEFINITIONS@", extra);
    }

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
    };

    std::string preloadFlags;
    preloadFlags += "\n --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd";
    for (const auto &[srcRel, dstRel] : preloadMap) {
        fs::path fullSrc = fs::path(baseDir) / srcRel;
        if (fs::exists(fullSrc)) {
            preloadFlags += " --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/" + srcRel + "@" + dstRel;
        }
    }

    if (p->LuaGuiObjects) {
        preloadFlags += " --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Gui@/Gui/";
    }

    // Generate the full CMake snippet
    PreloadedFiles += "set_target_properties(pd4web PROPERTIES LINK_FLAGS \"${EMCC_LINK_FLAGS}" +
                      preloadFlags + "\")\n";

    // Replace in template
    replaceAll(cmakeTemplate, "@PD4WEB_PRELOADED_PATCH@", PreloadedFiles);

    // Write cmake
    writeFile((p->WebPatchFolder / "CMakeLists.txt").string(), cmakeTemplate);
}

// ─────────────────────────────────────
void Pd4Web::createExternalsCppFile(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();
    print("Creating externals.cpp file", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
    print("\n");
    std::string externalsTemplate = readFile((p->Pd4WebFiles / "externals.in.cpp").string());

    std::string Declaration = "";
    std::string Call = "";

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
                              Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
                        Declaration += "extern \"C\" " +
                                       p->ExternalObjectsJson[pl.Lib]["objects"][pl.Name][0]
                                           .get<std::string>() +
                                       ";\n";
                        Call += "    " +
                                p->ExternalObjectsJson[pl.Lib]["objects"][pl.Name][1]
                                    .get<std::string>() +
                                "();\n";
                    }
                }
            }
        } else if (pl.isExternal && !pl.isLuaExternal && pl.Lib.empty() && !pl.Name.empty()) {
            if (p->ExternalObjectsJson.contains(pl.Name)) {
                if (p->ExternalObjectsJson[pl.Name]["objects"].contains(pl.Name)) {
                    print("Found " +
                              p->ExternalObjectsJson[pl.Name]["objects"][pl.Name][0]
                                  .get<std::string>() +
                              " setup function",
                          Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
                    Declaration +=
                        "extern \"C\" " +
                        p->ExternalObjectsJson[pl.Name]["objects"][pl.Name][0].get<std::string>() +
                        ";\n";
                    Call +=
                        "    " +
                        p->ExternalObjectsJson[pl.Name]["objects"][pl.Name][1].get<std::string>() +
                        "();\n";
                }
            }
        }
    }

    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_EXTRA@", extraDefinitions);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_DECLARATION@", Declaration);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_SETUP@", Call);
    writeFile((p->WebPatchFolder / "Pd4Web" / "externals.cpp").string(), externalsTemplate);
}

// ─────────────────────────────────────
void Pd4Web::configureProjectToCompile(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    print("\n");
    print("Configuring Build Project", Pd4WebLogLevel::PD4WEB_LOG1);

    createMainCmake(p);
    copySources(p);

    print("\n");
    print("Configuring C++ Code", Pd4WebLogLevel::PD4WEB_LOG1);
    createConfigFile(p);
    createExternalsCppFile(p);
}

// ─────────────────────────────────────
void Pd4Web::buildPatch(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    if (m_Error) {
        print("There are errors, solve them first!", Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    std::string buildType = m_Debug ? "Debug" : "Release";

    fs::path buildDir = p->WebPatchFolder / ".build";
    if (m_CleanBuild && fs::exists(buildDir)) {
        fs::remove_all(buildDir);
    }

    fs::create_directories(buildDir);

#if defined(_WIN32)
    DWORD attrs = GetFileAttributes(buildDir.string().c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        SetFileAttributes(buildDir.string().c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
    }
#endif

    if (!fs::exists(m_Ninja)) {
        print("Ninja binary not found at: " + m_Ninja, Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    // Step 1: Configure with emcmake
    std::vector<std::string> configureArgs = {m_Cmake,
                                              p->WebPatchFolder.string(),
                                              "-B",
                                              buildDir.string(),
                                              "-G",
                                              "Ninja",
                                              "-DPDCMAKE_DIR=Pd4Web/Externals/",
                                              "-DCMAKE_BUILD_TYPE=" + buildType,
                                              "-DEMCONFIGURE=" + m_Emconfigure,
                                              "-DEMMAKE=" + m_Emmake,
                                              "-DCMAKE_MAKE_PROGRAM=" + m_Ninja,
                                              "-Wno-dev"};

    int result = execProcess(m_Emcmake, configureArgs);
    if (result != 0) {
        print("Configuration step failed", Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    // Step 2: Build
    int cpuCount = std::thread::hardware_concurrency();
    std::vector<std::string> buildArgs = {"--build", buildDir.string(),
                                          "-j" + std::to_string(cpuCount), "--target", "pd4web"};

    result = execProcess(m_Cmake, buildArgs);
    if (result != 0) {
        print("Build failed", Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    print("Build completed successfully!", Pd4WebLogLevel::PD4WEB_LOG2);
}

// ─────────────────────────────────────
void Pd4Web::createAppManifest(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    fs::path webpatch = p->WebPatchFolder / "WebPatch";
    std::vector<std::string> fileList;
    for (const auto &entry : fs::directory_iterator(webpatch)) {
        if (fs::is_regular_file(entry)) {
            fileList.push_back(entry.path().filename().string());
        }
    }

    // Construct PWA manifest
    json manifest = {
        {"name", "WebPatch"},
        {"short_name", "WebPatch"},
        {"start_url", "index.html"},
        {"display", "standalone"},
        {"background_color", "#ffffff"},
        {"theme_color", "#000000"},
        {"icons",
         {{{"src", "icon-192.png"}, {"sizes", "192x192"}, {"type", "image/png"}},
          {{"src", "icon-512.png"}, {"sizes", "512x512"}, {"type", "image/png"}}}},
        {"files", fileList} // optional: not part of PWA spec
    };

    // Write manifest to disk
    fs::path manifestPath = webpatch / "manifest.json";
    std::ofstream out(manifestPath);
    out << manifest.dump(4); // pretty print with indent
    out.close();
}
