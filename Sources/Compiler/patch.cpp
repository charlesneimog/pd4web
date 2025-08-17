#include "pd4web_compiler.hpp"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>

// ─────────────────────────────────────
bool Pd4Web::openPatch(std::shared_ptr<Patch> &p) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    std::ifstream file(p->PathFile);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo: " << p->PathFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        PatchLine pl;
        pl.OriginalTokens = tokens;
        p->PatchLines.push_back(pl);
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processLine(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    std::vector<std::string> Line;
    Line = pl.OriginalTokens;
    if (Line.size() < 2) {
        return true;
    }

    if (Line[1] == "canvas") {
        pl.Type = PatchLine::CANVAS;
        p->CanvasLevel++;
    } else if (Line[1] == "restore") {
        pl.Type = PatchLine::RESTORE;
        p->CanvasLevel--;
    } else if (Line[1] == "declare") {
        pl.Type = PatchLine::DECLARE;
        processDeclareClass(p, pl);
    } else if (Line[1] == "obj") {
        processObjClass(p, pl);
        pl.Type = PatchLine::OBJ;
    } else if (Line[1] == "msg") {
        pl.Type = PatchLine::MSG;
    } else if (Line[1] == "connect") {
        pl.Type = PatchLine::CONNECTION;
    } else if (Line[1] == "text") {
        pl.Type = PatchLine::TEXT;
    } else if (Line[1] == "coords") {
        pl.Type = PatchLine::COORDS;
    } else {
        print("Class unknown " + Line[1], Pd4WebLogLevel::ERROR);
    }

    return true;
}

// ─────────────────────────────────────
fs::path Pd4Web::getAbsPath(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    bool found = false;

    std::string objName = pl.Name;
    if (pl.Name == "clone") {
        objName = pl.CloneAbs;
    }

    std::string fullPath = p->PatchFolder.string() + "/" + objName + ".pd";
    if (fs::exists(fullPath)) {
        return fullPath;
    }

    if (pl.Lib != "") {
        if (p->ExternalObjectsJson.contains(pl.Lib)) {
            if (p->ExternalObjectsJson[pl.Lib].contains("abstractions")) {
                if (p->ExternalObjectsJson[pl.Lib]["abstractions"].contains(objName)) {
                    fullPath = p->ExternalObjectsJson[pl.Lib]["abstractions"][objName][1];
                    if (fs::exists(fullPath)) {
                        return fullPath;
                    }
                }
            }
        }
    }

    for (auto Lib : p->DeclaredPaths) {
        if (Lib != "") {
            if (p->ExternalObjectsJson.contains(Lib)) {
                if (p->ExternalObjectsJson[Lib].contains("abstractions")) {
                    if (p->ExternalObjectsJson[Lib]["abstractions"].contains(objName)) {
                        fullPath = p->ExternalObjectsJson[Lib]["abstractions"][objName][1];
                        if (fs::exists(fullPath)) {
                            return fullPath;
                        }
                    }
                }
            }
        }
    }

    return fs::path("");
}

// ─────────────────────────────────────
void Pd4Web::isLuaObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    if (Patch->PdLua) {
        std::vector<fs::path> results;
        std::vector<std::string> subdirs = {"Extras", "Libs"};
        for (const auto &subdir : subdirs) {
            findLuaObjects(Patch, Patch->PatchFolder / subdir, pl);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isAbstraction(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    if (pl.Found) {
        return;
    }

    // with declared paths
    fs::path AbsPath;
    for (auto path : p->DeclaredPaths) {
        AbsPath = p->mainRoot / path / (pl.Name + ".pd");
        if (fs::exists(AbsPath)) {
            auto Abstraction = std::make_shared<Patch>();
            Abstraction->PathFile = AbsPath;
            Abstraction->PatchFolder = AbsPath.parent_path();

            print("\n");
            print("Processing clone SubPatch '" + Abstraction->PathFile.filename().string(),
                  Pd4WebLogLevel::LOG1, p->printLevel + 1);
            processSubpatch(p, Abstraction);

            pl.isAbstraction = true;
            pl.Found = true;
            return;
        }
    }

    // with preffix
    AbsPath = p->mainRoot / pl.Lib / (pl.Name + ".pd");
    if (fs::exists(AbsPath)) {
        auto Abstraction = std::make_shared<Patch>();
        Abstraction->PathFile = AbsPath;
        Abstraction->PatchFolder = AbsPath.parent_path();

        print("\n");
        print("Processing clone SubPatch '" + Abstraction->PathFile.filename().string(),
              Pd4WebLogLevel::LOG1, p->printLevel + 1);
        processSubpatch(p, Abstraction);
        pl.isAbstraction = true;
        pl.Found = true;
        return;
    }

    if (pl.Lib != "") {
        std::vector<std::string> abs = listAbstractionsInLibrary(p, pl.Lib);
        for (std::string obj : abs) {
            if (obj == pl.Name) {
                AbsPath = fs::path(p->ExternalObjectsJson[pl.Lib]["abstractions"][pl.Name][1]);
                if (fs::exists(AbsPath)) {
                    auto Abstraction = std::make_shared<Patch>();
                    Abstraction->PathFile = AbsPath;
                    Abstraction->PatchFolder = AbsPath.parent_path();

                    print("\n");
                    print("Processing clone SubPatch '" + Abstraction->PathFile.filename().string(),
                          Pd4WebLogLevel::LOG1, p->printLevel + 1);
                    processSubpatch(p, Abstraction);
                    pl.isAbstraction = true;
                    pl.Found = true;
                    return;
                }
            }
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isExternalLibObj(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    if (pl.Found) {
        return;
    }
    std::string libPreffix = pl.Lib;

    // object with preffix
    for (Library SupportedLib : m_Libraries) {
        if (SupportedLib.Name == pl.Lib && pl.Lib != "") {
            std::vector<std::string> objects = listObjectsInLibrary(p, pl.Lib);
            for (std::string obj : objects) {
                if (obj == pl.Name) {
                    print("Found external object '" + pl.Name +
                              "' via preffix lib: " + pl.OriginalTokens[4],
                          LOG2, p->printLevel + 1);
                    pl.isExternal = true;
                    pl.isAbstraction = false;
                    pl.isLuaExternal = false;
                    pl.Found = true;
                    p->DeclaredLibs.push_back(pl.Lib);
                    p->ExternalPatchLines.push_back(pl);
                    return;
                }
            }
        }
    }

    // objects that use declare Lib to declare lib (ex.: timbreLibId)
    for (std::string lib : p->DeclaredLibs) {
        std::vector<std::string> objects = listObjectsInLibrary(p, lib);
        for (std::string obj : objects) {
            if (obj == pl.Name) {
                print("Found external object '" + pl.Name + "' using lib in declared libs", LOG2,
                      p->printLevel + 1);
                pl.Found = true;
                pl.isExternal = true;
                pl.isLuaExternal = false;
                pl.Lib = lib;
                p->ExternalPatchLines.push_back(pl);
                return;
            }
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isPdObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    bool isPdObj = std::find(m_PdObjects.begin(), m_PdObjects.end(), pl.Name) != m_PdObjects.end();
    if (!isPdObj) {
        pl.isExternal = true;
        pl.Found = false;
        pl.isLuaExternal = false;
        return;
    }

    pl.isExternal = false;
    if (isNumber(pl.Name)) {
        pl.Found = true;
    }

    if (pl.Name[0] == '\\' && pl.Name[1] == '$') {
        pl.Found = true;
    }

    // check for midi and audio objects
    isMidiObj(Patch, pl);
}

// ─────────────────────────────────────
void Pd4Web::isMidiObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    std::vector<std::string> midiIn = {"notein", "cltin",          "bendin",
                                       "pgmin",  "touchin",        "polytouchin",
                                       "midiin", "midirealtimein", "sysexin"};

    std::vector<std::string> midiOut = {"noteout",  "cltout",       "bendout", "pgmout",
                                        "touchout", "polytouchout", "midiout"};

    if (std::find(midiIn.begin(), midiIn.end(), pl.Name) != midiIn.end()) {
        Patch->Midi = true;
        print("Activate Midi In Support", Pd4WebLogLevel::LOG2, Patch->printLevel + 1);
    } else if (std::find(midiOut.begin(), midiOut.end(), pl.Name) != midiOut.end()) {
        Patch->Midi = true;
        print("Activate Midi Out Support", Pd4WebLogLevel::LOG2, Patch->printLevel + 1);
    } else {
        Patch->Midi = false;
    }
}

// ─────────────────────────────────────
std::string Pd4Web::getObjLib(std::string &objToken) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    if (objToken == "/" || objToken == "//") {
        return "";
    }

    size_t slashPos = objToken.find_last_of('/');
    if (slashPos != std::string::npos) {
        return objToken.substr(0, slashPos);
    }

    return "";
}

// ─────────────────────────────────────
std::string Pd4Web::getObjName(std::string &objToken) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    if (objToken == "/" || objToken == "//") {
        return objToken;
    }

    std::string Obj = objToken;
    Obj.erase(std::remove(Obj.begin(), Obj.end(), ','), Obj.end());
    Obj.erase(std::remove(Obj.begin(), Obj.end(), ';'), Obj.end());

    size_t slashPos = Obj.find_last_of('/');
    if (slashPos != std::string::npos) {
        Obj = Obj.substr(slashPos + 1);
    }

    return Obj;
}

// ─────────────────────────────────────
bool Pd4Web::processObjAudioInOut(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    int length = pl.OriginalTokens.size();
    std::string Obj = getObjName(pl.OriginalTokens[4]);
    std::string Lib = getObjLib(pl.OriginalTokens[4]);

    if (Obj == "adc~") {
        unsigned int input = 0;

        if (length > 5) {
            for (size_t i = 5; i < pl.OriginalTokens.size(); ++i) {
                std::string token = pl.OriginalTokens[i];
                if (isNumber(token)) {
                    input = std::stoi(token);
                }
            }
        }

        if (!input) {
            input = 1;
        }
        p->Input = input;
        print("Number of input is " + std::to_string(input), Pd4WebLogLevel::LOG2,
              p->printLevel + 1);
    } else if (Obj == "dac~") {
        unsigned int output = 0;

        if (length > 5) {
            for (size_t i = 5; i < pl.OriginalTokens.size(); ++i) {
                std::string token = pl.OriginalTokens[i];
                if (isNumber(token)) {
                    output = std::stoi(token);
                }
            }
        }

        if (!output) {
            output = 1;
        }
        p->Output = output;
        print("Number of output is " + std::to_string(output), Pd4WebLogLevel::LOG2,
              p->printLevel + 1);
        return true;
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::processObjClone(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    std::unordered_set<std::string> args = {"#X", "obj", "clone", "-do", "-di", "-x", "-s", "f"};
    std::vector<std::string> filtered;
    std::copy_if(pl.OriginalTokens.begin(), pl.OriginalTokens.end(), std::back_inserter(filtered),
                 [&args](const std::string &token) {
                     auto is_number = [](std::string s) {
                         s.erase(std::remove_if(s.begin(), s.end(),
                                                [](char c) { return c == ',' || c == ';'; }),
                                 s.end());

                         return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
                     };

                     return args.find(token) == args.end() && !is_number(token);
                 });

    if (filtered.size() != 1) {
        for (auto filteredToken : filtered) {
            print(filteredToken, Pd4WebLogLevel::ERROR);
        }
        print("Filtered should return just 1 result, please report", Pd4WebLogLevel::ERROR);
        return false;
    } else {
        std::string abs = filtered[0];
        abs.erase(
            std::remove_if(abs.begin(), abs.end(), [](char c) { return c == ';' || c == ','; }),
            abs.end());

        pl.CloneAbs = abs;
        fs::path AbsPath = getAbsPath(p, pl);
        if (!fs::exists(AbsPath)) {
            print("AbsPath doesn't exists", Pd4WebLogLevel::ERROR);
            return false;
        }
        auto Abstraction = std::make_shared<Patch>();
        Abstraction->PathFile = AbsPath;
        Abstraction->PatchFolder = AbsPath.parent_path();
        pl.isAbstraction = true;
        print("\n");
        print("Processing clone SubPatch '" + abs + ".pd'", Pd4WebLogLevel::LOG1,
              p->printLevel + 1);
        processSubpatch(p, Abstraction);
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processObjClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    std::string Obj = getObjName(pl.OriginalTokens[4]);
    std::string Lib = getObjLib(pl.OriginalTokens[4]);
    if (libIsSupported(Lib)) {
        (void)downloadSupportedLib(Lib);
    }

    pl.Name = Obj;
    pl.Lib = Lib;
    if (Obj == "declare") {
        pl.isExternal = false;
        return true;
    } else if (Obj == "adc~" || Obj == "dac~") {
        pl.isExternal = false;
        processObjAudioInOut(p, pl);
        return true;
    } else if (Obj == "clone") {
        processObjClone(p, pl);
        pl.isExternal = false;
        return true;
    } else if (Obj == "pdlua") {
        p->PdLua = true;
        p->DeclaredLibs.push_back("pdlua");
        return true;
    }

    isPdObj(p, pl); // if not found it set isExternal to true

    if (pl.isExternal) {
        if (p->PdLua) {
            isLuaObj(p, pl);
        }

        isAbstraction(p, pl);
        isExternalLibObj(p, pl);

        if (!pl.Found) {
            print("Not Found object " + Obj, Pd4WebLogLevel::ERROR, p->printLevel + 1);
            return false;
        }
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processDeclareClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    size_t index = 2;
    while (index < pl.OriginalTokens.size()) {
        std::string Token = pl.OriginalTokens[index];
        std::string Lib = pl.OriginalTokens[index + 1];
        if (!Lib.empty() && Lib.back() == ';') {
            Lib.pop_back();
        }
        if (Token == "-lib") {
            if (libIsSupported(Lib)) {
                p->DeclaredLibs.push_back(Lib);
                std::vector<std::string> objectsInLibrary = listObjectsInLibrary(p, Lib);
                p->ValidObjectNames.insert(p->ValidObjectNames.end(), objectsInLibrary.begin(),
                                           objectsInLibrary.end());
                print("Found declare library " + Lib, Pd4WebLogLevel::LOG2, p->printLevel + 1);
                (void)downloadSupportedLib(Lib);
                if (Lib == "pdlua") {
                    p->PdLua = true;
                }
            } else {
                print("Declared lib " + Lib + " not supported", Pd4WebLogLevel::LOG2,
                      p->printLevel + 1);
            }

            // check if library is cloned

        } else if (Token == "-path") {
            p->DeclaredPaths.push_back(Lib);
            if (libIsSupported(Lib)) {
                p->DeclaredLibs.push_back(Lib);
                print("Found declare library " + Lib, Pd4WebLogLevel::LOG2, p->printLevel + 1);
            }
            print("Found declare path " + Lib, Pd4WebLogLevel::LOG2, p->printLevel + 1);
        }

        index += 2;
    }
    return true;
}

// ─────────────────────────────────────
void Pd4Web::updatePatch(std::shared_ptr<Patch> &p, bool mainPatch) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    std::string editPatch = "";
    for (auto pl : p->PatchLines) {

        // tradicional external
        if (pl.isExternal && pl.Type == PatchLine::OBJ) {
            std::string &token = pl.OriginalTokens[4];
            size_t firstSlash = token.find('/');
            if (firstSlash != std::string::npos) {
                size_t secondSlash = token.find('/', firstSlash + 1);
                if (secondSlash != std::string::npos) {
                    token = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                } else {
                    token = token.substr(firstSlash + 1);
                }
            }

            // abstraction but not clone
        } else if (pl.isAbstraction && pl.Type == PatchLine::OBJ && pl.Name != "clone") {
            std::string &token = pl.OriginalTokens[4];
            size_t firstSlash = token.find('/');
            if (firstSlash != std::string::npos) {
                size_t secondSlash = token.find('/', firstSlash + 1);
                if (secondSlash != std::string::npos) {
                    token = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                } else {
                    token = token.substr(firstSlash + 1);
                }
            }

            // clone abstraction
        } else if (pl.isAbstraction && pl.Type == PatchLine::OBJ && pl.Name == "clone") {
            std::unordered_set<std::string> args = {"#X", "obj", "clone", "-do", "-di", "-x", "-s"};

            std::vector<size_t> filteredIndexes;
            std::vector<std::string> filtered;

            for (size_t i = 0; i < pl.OriginalTokens.size(); ++i) {
                const std::string &token = pl.OriginalTokens[i];

                auto is_number = [](const std::string &s) {
                    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
                };

                if (args.find(token) == args.end() && !is_number(token)) {
                    std::string cleaned = token;

                    // Remove prefix before last slash, keep everything after (including ';')
                    size_t slashPos = cleaned.rfind('/');
                    if (slashPos != std::string::npos) {
                        cleaned = cleaned.substr(slashPos + 1);
                    }

                    filteredIndexes.push_back(i);
                    filtered.push_back(cleaned);
                }
            }

            for (size_t i = 0; i < filteredIndexes.size(); ++i) {
                pl.OriginalTokens[filteredIndexes[i]] = filtered[i];
            }
        }

        // check and replace gui objects
        if (pl.Type == PatchLine::OBJ) {
            std::unordered_set<std::string> guiObjs = {
                "vsl", "hsl", "vradio", "hradio", "tgl", "bng", "keyboard", "else/keyboard", "vu"};
            printf("canvas el: %d\n", p->CanvasLevel);
            if (guiObjs.count(pl.Name) && mainPatch && p->CanvasLevel == 1) {
                pl.OriginalTokens[4] = "l." + pl.Name;
                p->PdLua = true;
                p->LuaGuiObjects = true;
                print("Replacing Gui Object '" + pl.Name + "' with 'l." + pl.Name + "'",
                      Pd4WebLogLevel::LOG2, p->printLevel + 1);
            }
        }

        // save
        for (auto &token : pl.OriginalTokens) {
            editPatch += token + " ";
        }
        editPatch += "\n";
    }

    if (mainPatch) {
        print("\n");
        print("Saving " + p->PathFile.filename().string() + " as index.pd", Pd4WebLogLevel::LOG1);
        writeFile((p->WebPatchFolder / "WebPatch" / "index.pd").string(), editPatch);
    } else {
        print("Creating modified version of " + p->PathFile.filename().string(),
              Pd4WebLogLevel::LOG2, p->printLevel + 1);
        fs::create_directory(p->WebPatchFolder / ".tmp");
        writeFile((p->WebPatchFolder / ".tmp" / p->PathFile.filename()).string(), editPatch);
    }
}

// ─────────────────────────────────────
bool Pd4Web::processSubpatch(std::shared_ptr<Patch> &f, std::shared_ptr<Patch> &p) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    p->Father = f;
    p->mainRoot = f->mainRoot;
    p->DeclaredPaths = f->DeclaredPaths;
    p->DeclaredLibs = f->DeclaredLibs;
    p->printLevel = p->printLevel + 1;
    p->WebPatchFolder = f->WebPatchFolder;
    p->Pd4WebFiles = f->Pd4WebFiles;

    f->Childs.push_back(p);

    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    getSupportedLibraries(p);

    if (m_PdObjects.empty()) {
        print("Pd Objects should already be listed here", Pd4WebLogLevel::ERROR);
        return false;
    }

    unsigned int i = 0;
    for (PatchLine &Line : p->PatchLines) {
        ok = processLine(p, Line);
        if (!ok) {
            print("Failed to process subpatch line" + std::to_string(i), Pd4WebLogLevel::ERROR);
            return false;
        }
        i++;
    }

    // TODO: Review this
    for (auto lib : p->DeclaredLibs) {
        p->Father->DeclaredLibs.push_back(lib);
    }
    for (auto lib : p->DeclaredPaths) {
        p->Father->DeclaredPaths.push_back(lib);
    }

    for (auto &pl : p->PatchLines) {
        if (pl.isExternal) {
            p->Father->ExternalPatchLines.push_back(pl);
        }
    }

    updatePatch(p);

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::isUniqueObjFromLibrary(std::shared_ptr<Patch> &p, std::string &Obj) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    for (Library Lib : m_Libraries) {
        if (Lib.Name == Obj) {
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::compilePatch() {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    if (!m_Init) {
        print("Pd4Web init failed", Pd4WebLogLevel::ERROR);
        return false;
    }

    if (!fs::exists(m_PatchFile)) {
        if (m_PatchFile == "") {
            print("Please provide a patch file", Pd4WebLogLevel::ERROR);
        } else {
            print("Patch file " + m_PatchFile + " not found", Pd4WebLogLevel::ERROR);
        }
        return false;
    }

    auto p = std::make_shared<Patch>();
    p->PathFile = fs::path(m_PatchFile);
    p->PatchFolder = p->PathFile.parent_path();
    p->mainRoot = p->PatchFolder;
    p->Zoom = m_PatchZoom;
    p->PdVersion = m_PdVersion;

    if (m_Pd4WebFiles == "") {
        print("m_Pd4WebFiles not set", Pd4WebLogLevel::ERROR);
        return false;
    }

    p->Pd4WebFiles = m_Pd4WebFiles;
    p->PdLua = true;

    if (m_OutputFolder != "") {
        p->WebPatchFolder = fs::path(m_OutputFolder);
    } else {
        p->WebPatchFolder = p->PatchFolder;
    }
    p->ProjectName = p->PathFile.parent_path().filename().string();
    p->Pd4WebRoot = m_Pd4WebRoot;

    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    print("Getting PureData Internal Objects", Pd4WebLogLevel::VERBOSE, p->printLevel);
    getSupportedLibraries(p);

    if (m_PdObjects.empty()) {
        print("Listing PureData Objects", Pd4WebLogLevel::LOG2, p->printLevel + 1);
        m_PdObjects = listObjectsInLibrary(p, "pure-data/src");
        m_PdObjects.push_back("list");
    }
    p->ValidObjectNames = m_PdObjects;

    print("\n");
    print("Processing Patch '" + p->ProjectName + ".pd'", Pd4WebLogLevel::LOG1, p->printLevel);
    unsigned int i = 1;
    for (PatchLine &PatchLine : p->PatchLines) {
        ok = processLine(p, PatchLine);
        if (!ok) {
            print("Failed to process line " + std::to_string(i), Pd4WebLogLevel::ERROR,
                  p->printLevel + 1);
            return false;
        }

        i++;
    }

    fs::create_directory(p->WebPatchFolder / "WebPatch");
    fs::create_directory(p->WebPatchFolder / "Pd4Web");
    fs::create_directory(p->WebPatchFolder / "Pd4Web" / "Externals");
    fs::create_directory(p->WebPatchFolder / "Pd4Web" / "Gui");

    getSupportedLibraries(p);
    updatePatch(p, true);
    configureProjectToCompile(p);

    if (m_Error) {
        print("Errors found, fix them before compiling!", Pd4WebLogLevel::ERROR);
        return false;
    }

    print("\n");
    print("Start building Patch", Pd4WebLogLevel::LOG1);
    buildPatch(p);
    createAppManifest(p);
    if (!m_Error) {
        print("Finished", Pd4WebLogLevel::LOG1);
    }

    return true;
}
