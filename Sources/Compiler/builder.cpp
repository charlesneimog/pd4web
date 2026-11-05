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
    print("Creating pd4web_config.h file", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    std::string configFile = readFile((p->Pd4WebFiles / "config.in.h").string());
    std::string patchVersion = PD4WEB_VERSION_PATCH;

    // TODO: Update version
    replaceAll(configFile, "@PD4WEB_VERSION_MAJOR@", std::to_string(PD4WEB_VERSION_MAJOR));
    replaceAll(configFile, "@PD4WEB_VERSION_MINOR@", std::to_string(PD4WEB_VERSION_MINOR));
    replaceAll(configFile, "@PD4WEB_VERSION_PATCH@", "\"" + patchVersion + "\"");

    replaceAll(configFile, "@PD4WEB_PROJECT_NAME@", "\"" + p->ProjectName + "\"");

    replaceAll(configFile, "@PD4WEB_CHS_IN@", std::to_string(p->Input));
    replaceAll(configFile, "@PD4WEB_CHS_OUT@", std::to_string(p->Output < 1 ? 1 : p->Output));
    replaceAll(configFile, "@PD4WEB_SR@", std::to_string(p->Sr));

    replaceAll(configFile, "@PD4WEB_GUI@", p->RenderGui ? "1" : "0");
    replaceAll(configFile, "@PD4WEB_PATCH_ZOOM@", std::to_string(p->Zoom));
    replaceAll(configFile, "@PD4WEB_PATCH_WIDTH@", std::to_string(p->Width));
    replaceAll(configFile, "@PD4WEB_PATCH_HEIGHT@", std::to_string(p->Height));
    replaceAll(configFile, "@PD4WEB_PATCH_MARGINX@", std::to_string(p->MarginX));
    replaceAll(configFile, "@PD4WEB_PATCH_MARGINY@", std::to_string(p->MarginY));
    replaceAll(configFile, "@PD4WEB_PATCH_FONTSIZE@", std::to_string(p->FontSize));

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

    writeFile((p->OutputFolder / "Pd4Web" / "pd4web_config.h").string(), configFile);
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
             fs::copy_options::overwrite_existing);
    fs::copy(p->Pd4WebFiles / "pd4web.hpp", p->OutputFolder / "Pd4Web" / "pd4web.hpp",
             fs::copy_options::overwrite_existing);
    for (const char *source :
         {"RenderCommand.hpp", "RenderCommand.cpp", "RenderTransactionQueue.hpp",
          "ThorVGRenderer.hpp", "ThorVGRenderer.cpp"}) {
        fs::copy(p->Pd4WebFiles / source, p->OutputFolder / "Pd4Web" / source,
                 fs::copy_options::overwrite_existing);
    }
    fs::copy(p->Pd4WebFiles / "Libraries" / "thorvg.cmake",
             p->OutputFolder / "Pd4Web" / "thorvg.cmake", fs::copy_options::overwrite_existing);

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
                 fs::copy_options::recursive | fs::copy_options::overwrite_existing);
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

            fs::path Pd4WebCmake = p->Pd4WebFiles / "Libraries" / (LibName + ".cmake");
            if (fs::exists(Pd4WebCmake)) {
                fs::copy(Pd4WebCmake,
                         p->OutputFolder / "Pd4Web" / "Externals" / (LibName + ".cmake"),
                         fs::copy_options::skip_existing);
            } else {
                std::string LibCmake = "cmake_minimum_required(VERSION 3.25)\n\n";
                LibCmake += "project(" + LibName + ")\n";
                LibCmake += "set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})\n";
                LibCmake += "add_subdirectory(${LIB_DIR} EXCLUDE_FROM_ALL)\n";
                std::filesystem::path FilePath =
                    p->OutputFolder / "Pd4Web" / "Externals" / (LibName + ".cmake");
                std::ofstream OutFile(FilePath);
                if (OutFile.is_open()) {
                    OutFile << LibCmake;
                    OutFile.close();
                } else {
                    print("Failed to create cmake file for Library: " + LibName,
                          Pd4WebLogLevel::PD4WEB_ERROR);
                }
            }
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
    replaceAll(cmakeTemplate, "@LIBPD_TAG@", "0.56-2");

    // Lua (condicional)
    if (p->PdLua) {
        replaceAll(
            cmakeTemplate, "@PD4WEB_LUA_DEFS@",
            "add_definitions(-DPD4WEB_LUA)\n"
            "include_directories(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua/luas/lua\")");
    } else {
        replaceAll(cmakeTemplate, "@PD4WEB_LUA_DEFS@", "");
    }

    std::string LibrariesInclude = "";

    std::vector<std::string> AddedLibs;
    // pdlua is separeted to avoid conflict with else
    LibrariesInclude += "include(\"${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua.cmake\")\n";

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

    std::unordered_set<std::string> addedTargets;
    auto addTarget = [&](const std::string &name) {
        std::string str = name;
        size_t pos = 0;
        while ((pos = str.find('~', pos)) != std::string::npos) {
            str.replace(pos, 1, "_tilde");
            pos += 6;
        }
        if (addedTargets.insert(str).second) { // only add if not already present
            externalTargets += "\n" + std::string(32, ' ') + str;
        }
    };

    for (auto &pl : p->ExternalObjects) {
        if (pl.isExternal && !pl.isLuaExternal) {
            addTarget(pl.Name);
        }
    }

    for (auto &pl : p->ExtraObjects) {
        addTarget(pl.Name);
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
        if (!fs::exists(DeclaredPath) && fs::exists(fullSrc)) {
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

    std::unordered_set<std::string> emitted;

    for (const std::string &Lib : p->DeclaredLibs) {

        if (Lib == "cyclone") {
            if (emitted.insert("CYCLONE_OBJ_API").second) {
                extraDefinitions +=
                    "#if !defined(CYCLONE_OBJ_API)\n#define CYCLONE_OBJ_API\n#endif\n";
            }
        }

        if (Lib == "pmpd") {
            if (emitted.insert("PMPD_EXPORT").second) {
                extraDefinitions += "#if !defined(PMPD_EXPORT)\n#define PMPD_EXPORT\n#endif\n";
            }
        }
    }

    loadObjectsJson(p);
    std::vector<std::string> ExternalsAlreadyAdded;
    for (PatchLine &pl : p->ExternalObjects) {
        print("Finding setup function for external: '" + pl.Name + "'");

        if (pl.isAbstraction) {
            continue;
        }

        if (std::find(ExternalsAlreadyAdded.begin(), ExternalsAlreadyAdded.end(), pl.Name) !=
            ExternalsAlreadyAdded.end()) {
            continue;
        }

        if (pl.isExternal && !pl.isLuaExternal) {
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
            } else {
                printf("not found object inside externalsObjectsJson\n");
            }
        } else if (pl.isExternal && !pl.isLuaExternal) {
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
            } else {
                printf("not found object inside externalsObjectsJson\n");
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
    writeFile((p->OutputFolder / "Pd4Web" / "pd4web_externals.cpp").string(), externalsTemplate);
}

// ─────────────────────────────────────
void Pd4Web::configureProjectToCompile(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    print("\n");
    print("Configuring Build Project", Pd4WebLogLevel::PD4WEB_LOG1);

    copyLuaRequirements(p);
    createMainCmake(p);
    copySources(p);

    print("\n");
    print("Configuring C++ Code", Pd4WebLogLevel::PD4WEB_LOG1);
    createConfigFile(p);
    createExternalsCppFile(p);
}

// ─────────────────────────────────────
void Pd4Web::copyLuaRequirements(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    std::vector<fs::path> searchRoots;
    std::unordered_set<std::string> knownRoots;
    auto addSearchRoot = [&](const fs::path &root) {
        std::error_code ec;
        if (root.empty() || !fs::is_directory(root, ec)) {
            return;
        }

        fs::path normalized = fs::weakly_canonical(root, ec);
        if (ec) {
            normalized = root.lexically_normal();
        }
        if (knownRoots.insert(normalized.generic_string()).second) {
            searchRoots.push_back(normalized);
        }
    };

    addSearchRoot(p->PatchFolder / "Extras");
    addSearchRoot(p->PatchFolder / "Libs");
    for (const std::string &path : p->DeclaredPaths) {
        addSearchRoot(p->PatchFolder / path);
        addSearchRoot(p->Pd4WebRoot / path);
    }
    for (const std::string &lib : p->DeclaredLibs) {
        for (const fs::path &root : getLibrarySearchRoots(p, lib)) {
            addSearchRoot(root);
        }
    }
    addSearchRoot(p->PatchFolder);

    fs::path extras = p->OutputFolder / "Extras";
    std::unordered_set<std::string> copiedObjects;
    std::unordered_set<std::string> processedLuaFiles;
    std::unordered_set<std::string> processedModules;

    auto canonicalKey = [](const fs::path &path) {
        std::error_code ec;
        fs::path normalized = fs::weakly_canonical(path, ec);
        return (ec ? path.lexically_normal() : normalized).generic_string();
    };

    auto copyFile = [&](const fs::path &source, const fs::path &destination) {
        if (canonicalKey(source) == canonicalKey(destination)) {
            return true;
        }
        std::error_code ec;
        fs::create_directories(destination.parent_path(), ec);
        if (ec) {
            print("Failed to create Lua destination folder '" + destination.parent_path().string() +
                      "': " + ec.message(),
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            print("Failed to copy Lua file '" + source.string() + "': " + ec.message(),
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        print("Copying Lua file '" + source.filename().string() + "' to Extras",
              Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
        return true;
    };

    auto copyFolder = [&](const fs::path &source, const fs::path &destination) {
        if (canonicalKey(source) == canonicalKey(destination)) {
            return true;
        }
        std::error_code ec;
        fs::create_directories(destination, ec);
        if (ec) {
            print("Failed to create Lua destination folder '" + destination.string() +
                      "': " + ec.message(),
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        fs::copy(source, destination,
                 fs::copy_options::recursive | fs::copy_options::overwrite_existing |
                     fs::copy_options::skip_symlinks,
                 ec);
        if (ec) {
            print("Failed to copy Lua folder '" + source.string() + "': " + ec.message(),
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        print("Copying Lua folder '" + source.filename().string() + "' to Extras",
              Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
        return true;
    };

    auto luaObjectPathFromJson = [&](const std::string &library,
                                     const std::string &name) -> fs::path {
        if (!p->ExternalObjectsJson.contains(library)) {
            return {};
        }
        const auto &libraryEntry = p->ExternalObjectsJson[library];
        if (!libraryEntry.is_object() || !libraryEntry.contains("lua_objects")) {
            return {};
        }
        const auto &objects = libraryEntry["lua_objects"];
        if (!objects.is_object() || !objects.contains(name)) {
            return {};
        }
        const auto &entry = objects[name];
        if (entry.is_array() && entry.size() > 1 && entry[1].is_string()) {
            fs::path candidate = entry[1].get<std::string>();
            if (fs::is_regular_file(candidate)) {
                return candidate;
            }
        }
        return {};
    };

    auto findObjectInJson = [&](const PatchLine &obj) -> fs::path {
        if (!p->ExternalObjectsJson.is_object()) {
            return {};
        }
        if (!obj.Lib.empty()) {
            fs::path candidate = luaObjectPathFromJson(obj.Lib, obj.Name);
            if (!candidate.empty()) {
                return candidate;
            }
        }
        for (auto library = p->ExternalObjectsJson.begin(); library != p->ExternalObjectsJson.end();
             ++library) {
            fs::path candidate = luaObjectPathFromJson(library.key(), obj.Name);
            if (!candidate.empty()) {
                return candidate;
            }
        }
        return {};
    };

    auto findLuaObject = [&](const PatchLine &obj) -> fs::path {
        fs::path jsonPath = findObjectInJson(obj);
        if (!jsonPath.empty()) {
            return jsonPath;
        }

        const fs::path filename = obj.Name + ".pd_lua";
        const std::string patchFolderKey = canonicalKey(p->PatchFolder);
        for (const fs::path &root : searchRoots) {
            fs::path direct = root / filename;
            if (fs::is_regular_file(direct)) {
                return direct;
            }
            if (canonicalKey(root) == patchFolderKey) {
                continue;
            }

            std::error_code ec;
            fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied,
                                                ec);
            fs::recursive_directory_iterator end;
            while (!ec && it != end) {
                if (it->is_regular_file(ec) && it->path().filename() == filename) {
                    return it->path();
                }
                it.increment(ec);
            }
        }
        return {};
    };

    struct LuaModule {
        fs::path File;
        fs::path Root;
        fs::path Relative;
    };

    auto isSafeModulePath = [](const fs::path &path) {
        if (path.empty() || path.is_absolute() || path.has_root_name()) {
            return false;
        }
        for (const fs::path &part : path) {
            if (part == "..") {
                return false;
            }
        }
        return true;
    };

    auto findModule = [&](const std::string &requirement, const fs::path &requiringFolder,
                          const fs::path &objectRoot) -> LuaModule {
        std::string moduleName = requirement;
        std::replace(moduleName.begin(), moduleName.end(), '\\', '/');
        if (moduleName.ends_with(".lua")) {
            moduleName.resize(moduleName.size() - 4);
        }

        std::vector<fs::path> modulePaths = {fs::path(moduleName)};
        std::string dotted = moduleName;
        std::replace(dotted.begin(), dotted.end(), '.', '/');
        if (dotted != moduleName) {
            modulePaths.emplace_back(dotted);
        }

        std::vector<fs::path> roots;
        std::unordered_set<std::string> moduleRoots;
        auto addModuleRoot = [&](const fs::path &root) {
            if (!root.empty() && moduleRoots.insert(canonicalKey(root)).second) {
                roots.push_back(root);
            }
        };
        addModuleRoot(requiringFolder);
        addModuleRoot(objectRoot);
        for (const fs::path &root : searchRoots) {
            addModuleRoot(root);
        }

        for (const fs::path &modulePath : modulePaths) {
            if (!isSafeModulePath(modulePath)) {
                continue;
            }
            for (const fs::path &root : roots) {
                for (const fs::path &relative :
                     {fs::path(modulePath.string() + ".lua"), modulePath / "init.lua"}) {
                    fs::path candidate = root / relative;
                    if (fs::is_regular_file(candidate)) {
                        return {candidate, root, relative};
                    }
                }
            }
        }
        return {};
    };

    std::function<void(const fs::path &, const fs::path &)> processRequirements;
    processRequirements = [&](const fs::path &luaFile, const fs::path &objectRoot) {
        if (!processedLuaFiles.insert(canonicalKey(luaFile)).second) {
            return;
        }

        for (const std::string &requirement : GetLuaRequireFolder(luaFile.string())) {
            LuaModule module = findModule(requirement, luaFile.parent_path(), objectRoot);
            if (module.File.empty()) {
                print("Lua requirement '" + requirement + "' referenced by '" +
                          luaFile.filename().string() + "' was not found",
                      Pd4WebLogLevel::PD4WEB_WARNING, p->printLevel + 1);
                continue;
            }

            std::string moduleKey = requirement + "@" + canonicalKey(module.Root);
            if (!processedModules.insert(moduleKey).second) {
                continue;
            }

            auto relative = module.Relative.lexically_normal();
            auto first = relative.begin();
            bool directoryModule = first != relative.end() && std::next(first) != relative.end();
            if (directoryModule) {
                fs::path sourceFolder = module.Root / *first;
                fs::path destinationFolder = extras / *first;
                if (!copyFolder(sourceFolder, destinationFolder)) {
                    continue;
                }

                std::error_code ec;
                fs::recursive_directory_iterator it(
                    sourceFolder, fs::directory_options::skip_permission_denied, ec);
                fs::recursive_directory_iterator end;
                while (!ec && it != end) {
                    if (it->is_regular_file(ec)) {
                        const std::string extension = it->path().extension().string();
                        if (extension == ".lua" || extension == ".pd_lua") {
                            processRequirements(it->path(), module.Root);
                        }
                    }
                    it.increment(ec);
                }
            } else {
                fs::path destination = extras / module.Relative;
                if (copyFile(module.File, destination)) {
                    processRequirements(module.File, module.Root);
                }
            }
        }
    };

    auto copyObjectAndRequirements = [&](const PatchLine &obj) {
        if (!obj.isLuaExternal || !copiedObjects.insert(obj.Name).second) {
            return;
        }

        fs::path source = findLuaObject(obj);
        if (source.empty()) {
            print("Lua object '" + obj.Name +
                      ".pd_lua' was detected but its source file "
                      "could not be found",
                  Pd4WebLogLevel::PD4WEB_ERROR, p->printLevel + 1);
            return;
        }

        if (copyFile(source, extras / source.filename())) {
            fs::path objectRoot = source.parent_path();
            processRequirements(source, objectRoot);
        }
    };

    // Main-patch Lua objects live in PatchLines. Lua objects inherited from
    // abstractions are collected in ExternalObjects by processSubpatch().
    for (const PatchLine &obj : p->PatchLines) {
        copyObjectAndRequirements(obj);
    }
    for (const PatchLine &obj : p->ExternalObjects) {
        copyObjectAndRequirements(obj);
    }
}

// ─────────────────────────────────────
void Pd4Web::copyExtraSources(std::shared_ptr<Patch> &p, fs::path buildDir) {
    // pd.cmake
    fs::path pdcmake = p->Pd4WebRoot / "pd.cmake" / "pd.cmake";

    fs::path destDir = p->OutputFolder / ".build";
    fs::create_directories(destDir);
    fs::copy(pdcmake, destDir, fs::copy_options::skip_existing);
    print("pd.cmake to " + (p->OutputFolder / ".build").string());

    // ThorVG
    fs::path thorvg = p->Pd4WebRoot / "thorvg";
    fs::path dest = p->OutputFolder / "Pd4Web" / "thorvg";
    fs::create_directories(dest);
    fs::copy(thorvg, dest, fs::copy_options::recursive | fs::copy_options::skip_existing);
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
                                        "-DCPM_SOURCE_CACHE=" +
                                            (p->Pd4WebRoot / "Dependencies").string(),
                                        "-DPD4WEB_AS_ES6=" + std::to_string(m_ExportES6Module),
                                        "-Wno-dev"};

    int result = execProcess(m_Emcmake.string(), cfgArgs);
    if (result != 0) {
        print("Configuration step failed, try to use the flag '--clear-before-compile' or "
              "configure to clear the configuration before compile. WARNING: This will remove "
              "WebPatch, Pd4Web, and .build",
              Pd4WebLogLevel::PD4WEB_ERROR);
        return;
    }

    // Step 2: Build
    int cpuCount = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) / 2);
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
