#include "pd4web_compiler.hpp"

#include <string>
#include <thread>
#include <fstream>

#define BOOST_PROCESS_VERSION 2
#include <boost/process.hpp>
#include <boost/asio.hpp>

// ─────────────────────────────────────
void Pd4Web::createConfigFile(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();
    print("Creating config.h file", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    std::string configFile = readFile((p->Pd4WebFiles / "config.in.h").string());

    // TODO: Update version
    replaceAll(configFile, "@PD4WEB_VERSION_MAJOR@", std::to_string(PD4WEB_VERSION_MAJOR));
    replaceAll(configFile, "@PD4WEB_VERSION_MINOR@", std::to_string(PD4WEB_VERSION_MINOR));
    replaceAll(configFile, "@PD4WEB_VERSION_PATCH@", PD4WEB_VERSION_PATCH);

    replaceAll(configFile, "@PD4WEB_PROJECT_NAME@", "\"" + p->ProjectName + "\"");

    replaceAll(configFile, "@PD4WEB_CHS_IN@", std::to_string(p->Input));
    replaceAll(configFile, "@PD4WEB_CHS_OUT@", std::to_string(p->Output < 1 ? 1 : p->Output));
    replaceAll(configFile, "@PD4WEB_SR@", std::to_string(p->Sr));

    replaceAll(configFile, "@PD4WEB_GUI@", p->RenderGui ? "1" : "0");
    replaceAll(configFile, "@PD4WEB_PATCH_ZOOM@", std::to_string(p->Zoom));
    replaceAll(configFile, "@PD4WEB_MIDI@", std::to_string(p->Midi));

    // number input
    std::string pd4web_number_input = "static const char *PD4WEB_NUMBER_INPUT[] = {";
    for (auto in : m_NumberInput) {
        pd4web_number_input += "\"" + in + "\",";
    }
    pd4web_number_input += "};\n";
    pd4web_number_input +=
        "static int PD4WEB_NUMBER_INPUT_SIZE = " + std::to_string(m_NumberInput.size()) + ";\n\n";
    replaceAll(configFile, "@PD4WEB_NUMBER_INPUT@", pd4web_number_input);

    // keyboard
    std::string pd4web_qwerty_input = "static const char* PD4WEB_QWERTY_INPUT[] = {";
    for (auto in : m_QwertyInput) {
        pd4web_qwerty_input += "\"" + in + "\",";
    }
    pd4web_qwerty_input += "};\n";
    pd4web_qwerty_input +=
        "static int PD4WEB_QWERTY_INPUT_SIZE = " + std::to_string(m_QwertyInput.size()) + ";\n\n";
    replaceAll(configFile, "@PD4WEB_QWERTY_INPUT@", pd4web_qwerty_input);

    writeFile((p->OutputFolder / "Pd4Web" / "config.h").string(), configFile);
}

// ─────────────────────────────────────
void Pd4Web::copySources(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();
    fs::copy(p->Pd4WebFiles / "Libraries" / "libpd.cmake",
             p->OutputFolder / "Pd4Web" / "libpd.cmake", fs::copy_options::skip_existing);

    fs::create_directory(p->OutputFolder / "Pd4Web" / "pure-data");

    print("Copying pure-data sources to Pd4Web", Pd4WebLogLevel::PD4WEB_LOG2, 2);
    fs::copy(p->Pd4WebRoot / "pure-data" / "src", p->OutputFolder / "Pd4Web" / "pure-data" / "src",
             fs::copy_options::recursive | fs::copy_options::skip_existing |
                 fs::copy_options::skip_symlinks);

    fs::copy(p->Pd4WebFiles / "pd4web.cpp", p->OutputFolder / "Pd4Web" / "pd4web.cpp",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.hpp", p->OutputFolder / "Pd4Web" / "pd4web.hpp",
             fs::copy_options::skip_existing);

    // Pd Lua
    if (p->PdLua) {
        if (!fs::exists(p->OutputFolder / "Pd4Web" / "Externals" / "pdlua")) {
            fs::copy(p->Pd4WebRoot / "pdlua", p->OutputFolder / "Pd4Web" / "Externals" / "pdlua",
                     fs::copy_options::recursive);
        }

        print("Replacig lua sources from pdlua to pd4web", Pd4WebLogLevel::PD4WEB_LOG2, 2);

        fs::copy(p->Pd4WebFiles / "pd4weblua.c",
                 p->OutputFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua.c",
                 fs::copy_options::overwrite_existing);
        fs::copy(p->Pd4WebFiles / "pd4weblua.h",
                 p->OutputFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua.h",
                 fs::copy_options::overwrite_existing);
        fs::copy(p->Pd4WebFiles / "pd4weblua_gfx.c",
                 p->OutputFolder / "Pd4Web" / "Externals" / "pdlua" / "pdlua_gfx.h",
                 fs::copy_options::overwrite_existing);

        fs::copy(p->Pd4WebFiles / "InterRegular.ttf",
                 p->OutputFolder / "Pd4Web" / "Externals" / "pdlua" / "InterRegular.ttf",
                 fs::copy_options::skip_existing);
    }

    writeFile((p->OutputFolder / "index.html").string(),
              "<!doctype html><meta http-equiv=\"refresh\" content=\"0;url=WebPatch\">");

    // Lua Gui Objects
    if (p->LuaGuiObjects) {
        fs::copy(p->Pd4WebFiles / "Gui", p->OutputFolder / "Pd4Web" / "Gui",
                 fs::copy_options::recursive | fs::copy_options::skip_existing);
    }

    // JavaScript
    fs::copy(p->Pd4WebFiles / "favicon.ico", p->OutputFolder / "favicon.ico",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.sw.js", p->OutputFolder / "WebPatch" / "pd4web.sw.js",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "icon-512.png", p->OutputFolder / "WebPatch" / "icon-512.png",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "icon-192.png", p->OutputFolder / "WebPatch" / "icon-192.png",
             fs::copy_options::skip_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.threads.js",
             p->OutputFolder / "WebPatch" / "pd4web.threads.js", fs::copy_options::skip_existing);
}

// ──────────────────────────────────────────
void Pd4Web::copyCmakeLibFiles(std::shared_ptr<Patch> &p, std::string LibName) {
    PD4WEB_LOGGER();
    for (Library Lib : m_Libraries) {
        fs::path cmakeFile = p->OutputFolder / "Pd4Web" / "Externals" / (LibName + ".cmake");
        if (Lib.Name == LibName && !fs::exists(cmakeFile)) {
            print("Copying files of '" + LibName + "' to Pd4Web/Externals",
                  Pd4WebLogLevel::PD4WEB_LOG2, 2);
            fs::copy(p->Pd4WebRoot / LibName, p->OutputFolder / "Pd4Web" / "Externals" / LibName,
                     fs::copy_options::recursive | fs::copy_options::skip_existing |
                         fs::copy_options::skip_symlinks);

            print("Copying build script '" + LibName + ".cmake' to Pd4Web/Externals",
                  Pd4WebLogLevel::PD4WEB_LOG2, 2);

            fs::copy(p->Pd4WebFiles / "Libraries" / (LibName + ".cmake"),
                     p->OutputFolder / "Pd4Web" / "Externals" / (LibName + ".cmake"),
                     fs::copy_options::skip_existing);
        }
    }

    if (p->PdLua) {
        fs::copy(p->Pd4WebFiles / "Libraries" / "pdlua.cmake",
                 p->OutputFolder / "Pd4Web" / "Externals" / "pdlua.cmake",
                 fs::copy_options::skip_existing);
    }
}

// ─────────────────────────────────────
void Pd4Web::createMainCmake(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    //
    print("Configuring CMakeLists.txt", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    fs::create_directory(p->OutputFolder / "Pd4Web");
    fs::create_directory(p->OutputFolder / "Pd4Web" / "Externals");
    fs::create_directory(p->OutputFolder / "WebPatch");

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

    for (auto &pl : p->ExternalObjects) {
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

    for (auto &pl : p->ExtraObjects) {
        std::string str = pl.Name;
        size_t pos = 0;
        while ((pos = str.find('~', pos)) != std::string::npos) {
            str.replace(pos, 1, "_tilde");
            pos += 6;
        }
        externalTargets += "\n" + std::string(32, ' ') + str;
    }

    std::string ExternalsTargets = "target_link_libraries(pd4web PRIVATE " + externalTargets + ")";
    replaceAll(cmakeTemplate, "@PD4WEB_EXTERNAL_OBJECTS_TARGET@", ExternalsTargets);

    // Preload files
    std::string PreloadedFiles = "get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)\n"
                                 "if(NOT EMCC_LINK_FLAGS)\n"
                                 "    set(EMCC_LINK_FLAGS \"\")\n"
                                 "endif()\n";

    // Collect all --preload-file entries into one string
    std::string baseDir = p->OutputFolder.string();
    std::vector<std::pair<std::string, std::string>> preloadMap = {
        {".tmp", "/Libs/"},
        {"Audios", "/Audios/"},
        {"Extras", "/Extras/"},
    };

    std::string preloadFlags;

    // index.pd
    preloadFlags +=
        "\n\t--preload-file \\\"${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd\\\"";

    // additional preload files
    for (const auto &[srcRel, dstRel] : preloadMap) {
        fs::path fullSrc = fs::path(baseDir) / srcRel;
        if (fs::exists(fullSrc)) {
            preloadFlags += "\n\t--preload-file \\\"${CMAKE_CURRENT_SOURCE_DIR}/" + srcRel + "@" +
                            dstRel + "\\\"";
        }
    }

    for (auto DeclaredPath : p->DeclaredPaths) {
        fs::path fullSrc = fs::path(baseDir) / DeclaredPath;
        if (fs::exists(fullSrc)) {
            preloadFlags += "\n\t--preload-file \\\"${CMAKE_CURRENT_SOURCE_DIR}/" + DeclaredPath +
                            "@" + DeclaredPath + "\\\"";
        }
    }

    // optional Lua GUI
    if (p->LuaGuiObjects) {
        preloadFlags += "\n\t--preload-file \\\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Gui@/Gui/\\\"";
    }

    // generate CMake snippet
    PreloadedFiles += "set_target_properties(pd4web PROPERTIES LINK_FLAGS \"${EMCC_LINK_FLAGS}" +
                      preloadFlags + "\")\n";

    // Replace in template
    replaceAll(cmakeTemplate, "@PD4WEB_PRELOADED_PATCH@", PreloadedFiles);

    // Write cmake
    writeFile((p->OutputFolder / "CMakeLists.txt").string(), cmakeTemplate);
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

    std::vector<std::string> ExternalsAlreadyAdded;
    for (PatchLine &pl : p->ExternalObjects) {
        if (pl.isAbstraction && pl.Type != PatchLine::OBJ && pl.Tokens[1] != "obj") {
            continue;
        }

        if (std::find(ExternalsAlreadyAdded.begin(), ExternalsAlreadyAdded.end(), pl.Name) !=
            ExternalsAlreadyAdded.end()) {
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
        ExternalsAlreadyAdded.push_back(pl.Name);
    }

    for (PatchLine &pl : p->ExtraObjects) {
        std::string setupName = pl.Name;
        size_t pos = 0;
        while ((pos = setupName.find("~", pos)) != std::string::npos) {
            setupName.replace(pos, 1, "_tilde");
            pos += 6;
        }
        Declaration += "extern \"C\" void " + setupName + "_setup(void);\n";
        Call += "    " + setupName + "_setup();\n";
    }

    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_EXTRA@", extraDefinitions);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_DECLARATION@", Declaration);
    replaceAll(externalsTemplate, "@PD4WEB_EXTERNAL_SETUP@", Call);
    writeFile((p->OutputFolder / "Pd4Web" / "externals.cpp").string(), externalsTemplate);
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
void Pd4Web::copyExtraSources(std::shared_ptr<Patch> &p, fs::path buildDir) {
    // pd.cmake
    fs::path pdcmake = p->Pd4WebRoot / "pd.cmake" / "pd.cmake";
    fs::copy(pdcmake, p->OutputFolder / "Pd4Web", fs::copy_options::skip_existing);
    print("pd.cmake to " + p->OutputFolder.string() + "Pd4Web");

    // nanovg
    fs::path nanovg = p->Pd4WebRoot / "nanovg";
    fs::path dest = p->OutputFolder / "Pd4Web" / "nanovg";
    fs::create_directories(dest);
    fs::copy(nanovg, dest, fs::copy_options::recursive | fs::copy_options::skip_existing);
}

// ─────────────────────────────────────
void Pd4Web::buildPatch(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    if (m_Error) {
        print("There are errors, solve them first!", Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    std::string buildType = m_Debug ? "Debug" : "Release";

    fs::path buildDir = p->OutputFolder / ".build";
    if (m_CleanBuild && fs::exists(buildDir)) {
        fs::remove_all(buildDir);
    }
    fs::create_directories(buildDir);

    copyExtraSources(p, buildDir);

#if defined(_WIN32)
    DWORD attrs = GetFileAttributes(buildDir.string().c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        SetFileAttributes(buildDir.string().c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
    }
#endif

    if (!fs::exists(m_Ninja)) {
        print("Ninja binary not found at: " + m_Ninja.string(), Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    // Step 1: Configure with emcmake
    std::vector<std::string> cfgArgs = {m_Cmake.string(),
                                        p->OutputFolder.string(),
                                        "-B",
                                        buildDir.string(),
                                        "-G",
                                        "Ninja",
                                        "-DCMAKE_BUILD_TYPE=" + buildType,
                                        "-DCMAKE_MAKE_PROGRAM=" + m_Ninja.string(),
                                        "-DPD4WEB_AS_ES6=" + std::to_string(m_ExportES6Module),
                                        "-Wno-dev"};

    int result = execProcess(m_Emcmake.string(), cfgArgs);
    if (result != 0) {
        print("Configuration step failed", Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    // Step 2: Build
    int cpuCount = std::thread::hardware_concurrency();
    std::vector<std::string> buildArgs = {"--build", buildDir.string(),
                                          "-j" + std::to_string(cpuCount), "--target", "pd4web"};

    result = execProcess(m_Cmake.string(), buildArgs);
    if (result != 0) {
        print("Build failed", Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    print("Build completed successfully!", Pd4WebLogLevel::PD4WEB_LOG2);
}

// ─────────────────────────────────────
void Pd4Web::createAppManifest(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    fs::path webpatch = p->OutputFolder / "WebPatch";
    std::vector<std::string> fileList;
    for (const auto &entry : fs::directory_iterator(webpatch)) {
        if (fs::is_regular_file(entry)) {
            fileList.push_back(entry.path().filename().string());
        }
    }

    /*
        id
    */

    // Construct PWA manifest
    json manifest = {
        {"name", p->ProjectName},
        {"short_name", p->ProjectName},
        {"start_url", "index.html"},
        {"display", "standalone"},
        {"background_color", "#ffffff"},
        {"theme_color", "#000000"},
        {"screenshots", ""},
        {"description", ""},
        {"display", "standalone"},
        {"icons",
         {{{"src", "icon-192.png"}, {"sizes", "192x192"}, {"type", "image/png"}},
          {{"src", "icon-512.png"}, {"sizes", "512x512"}, {"type", "image/png"}}}},
        {"orientation", "any"},
    };

    // Write manifest to disk
    fs::path manifestPath = webpatch / "manifest.json";
    std::ofstream out(manifestPath);
    out << manifest.dump(4); // pretty print with indent
    out.close();
}
