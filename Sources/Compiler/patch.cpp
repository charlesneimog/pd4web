#include "pd4web_compiler.hpp"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>

// ─────────────────────────────────────
bool Pd4Web::openPatch(std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();
    std::ifstream file(p->PatchFile);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo: " << p->PatchFile << std::endl;
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
        pl.Tokens = tokens;
        pl.Line = line;
        p->PatchLines.push_back(pl);
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processLine(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
    if (m_Error) {
        return false;
    }

    std::vector<std::string> Line = pl.Tokens;
    if (Line.size() < 2) {
        return true;
    }

    if (Line[0] == "#X") {
        m_inArray = false;
        if (Line[1] == "restore") {
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
        } else if (Line[1] == "array") {
            pl.Type = PatchLine::ARRAY;
        } else if (Line[1] == "connect") {
            pl.Type = PatchLine::CONNECTION;
        } else if (Line[1] == "text") {
            pl.Type = PatchLine::TEXT;
        } else if (Line[1] == "coords") {
            pl.Type = PatchLine::COORDS;
        } else if (Line[1] == "floatatom") {
            pl.Type = PatchLine::FLOATATOM;
            processCanvasAtoms(p, pl);
        } else if (Line[1] == "symbolatom") {
            pl.Type = PatchLine::SYMBOLATOM;
            processCanvasAtoms(p, pl);
        } else if (Line[1] == "listbox") {
            pl.Type = PatchLine::LISTATOM;
            processCanvasAtoms(p, pl);
        } else if (Line[1] == "f") {
            pl.Type = PatchLine::FLOAT;
        } else {
            print("t_canvas class unknown " + Line[1], Pd4WebLogLevel::PD4WEB_ERROR);
        }
    } else if (Line[0] == "#A") {
        m_inArray = true;
    } else if (m_inArray) {
        // Nothing To-Do;
    } else if (Line[0] == "#N") {
        // Reative o CanvasLevel se você precisa que a troca de GUI funcione
        p->CanvasLevel++;
    } else if (Line[0][0] == '#') {
        print("Class unknown " + Line[0] + " in " + p->PatchFile.string(),
              Pd4WebLogLevel::PD4WEB_WARNING);
    } else {
        // Unknown, ignore
    }

    return true;
}

// ─────────────────────────────────────
fs::path Pd4Web::getAbsPath(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
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

    for (auto Path : p->DeclaredPaths) {
        if (Path != "") {
            // if is something like declare -path else
            if (p->ExternalObjectsJson.contains(Path)) {
                if (p->ExternalObjectsJson[Path].contains("abstractions")) {
                    if (p->ExternalObjectsJson[Path]["abstractions"].contains(objName)) {
                        fullPath = p->ExternalObjectsJson[Path]["abstractions"][objName][1];
                        if (fs::exists(fullPath)) {
                            return fullPath;
                        }
                    }
                }
            } else if (fs::exists(p->PatchFolder / Path / (objName + ".pd"))) {
                return p->PatchFolder / Path / (objName + ".pd");
            }
        }
    }

    for (auto Lib : p->DeclaredLibs) {
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

    size_t pos = objName.find('/');
    if (pos != std::string::npos) {
        std::string Lib = objName.substr(0, pos);
        objName = objName.substr(pos + 1);
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

    print("Abstraction '" + objName + ".pd' not found", Pd4WebLogLevel::PD4WEB_ERROR);
    return fs::path("");
}

// ─────────────────────────────────────
void Pd4Web::isLuaObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    PD4WEB_LOGGER();
    if (Patch->PdLua) {
        std::vector<fs::path> results;
        std::vector<std::string> subdirs = {"Extras", "Libs"};
        for (const auto &subdir : subdirs) {
            findLuaObjects(Patch, Patch->PatchFolder / subdir, pl);
            if (pl.Found) {
                break;
            }
        }
        for (const auto &subdir : Patch->DeclaredPaths) {
            findLuaObjects(Patch, Patch->PatchFolder / subdir, pl);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isAbstraction(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
    if (pl.Found) {
        return;
    }

    // with declared paths
    fs::path AbsPath;
    for (auto path : p->DeclaredPaths) {
        AbsPath = p->PatchFolder / path / (pl.Name + ".pd");
        if (fs::exists(AbsPath)) {
            auto Abstraction = std::make_shared<Patch>();
            Abstraction->PatchFile = AbsPath;
            Abstraction->PatchFolder = AbsPath.parent_path();
            Abstraction->Pd4WebRoot = p->Pd4WebRoot;
            pl.isAbstraction = true;
            pl.isExternal = false;

            print("\n");
            print("Processing clone SubPatch '" + Abstraction->PatchFile.filename().string(),
                  Pd4WebLogLevel::PD4WEB_LOG1, p->printLevel + 1);
            processSubpatch(p, Abstraction);

            pl.Found = true;
            return;
        }
    }

    // with prefix
    AbsPath = p->PatchFolder / pl.Lib / (pl.Name + ".pd");
    if (fs::exists(AbsPath)) {
        auto Abstraction = std::make_shared<Patch>();
        Abstraction->PatchFile = AbsPath;
        Abstraction->PatchFolder = AbsPath.parent_path();
        pl.isAbstraction = true;
        pl.isExternal = false;

        print("\n");
        print("Processing clone SubPatch '" + Abstraction->PatchFile.filename().string(),
              Pd4WebLogLevel::PD4WEB_LOG1, p->printLevel + 1);
        processSubpatch(p, Abstraction);
        pl.Found = true;
        return;
    }

    if (pl.Lib != "") {
        std::vector<std::string> abs = listAbstractionsInLibrary(p, pl.Lib);
        if (abs.empty()) {
            print("No abstractions found in library " + pl.Lib, Pd4WebLogLevel::PD4WEB_ERROR);
            return;
        }
        for (std::string obj : abs) {
            if (obj == pl.Name) {
                AbsPath = fs::path(
                    p->ExternalObjectsJson[pl.Lib]["abstractions"][pl.Name][1].get<std::string>());

                if (fs::exists(AbsPath)) {
                    auto Abstraction = std::make_shared<Patch>();
                    Abstraction->PatchFile = AbsPath;
                    Abstraction->PatchFolder = AbsPath.parent_path();
                    pl.isAbstraction = true;
                    pl.isExternal = false;

                    print("\n");
                    print("Processing clone SubPatch '" +
                              Abstraction->PatchFile.filename().string(),
                          Pd4WebLogLevel::PD4WEB_LOG1, p->printLevel + 1);
                    processSubpatch(p, Abstraction);
                    pl.Found = true;
                    return;
                } else {
                    print("Abstraction '" + pl.Name + ".pd' not found in library " + pl.Lib,
                          Pd4WebLogLevel::PD4WEB_ERROR);
                }
            }
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isExtraObj(std::shared_ptr<Patch> &p, PatchLine &pl) {
    std::string libprefix = pl.Lib;

    std::vector<std::string> CompiledObjects = {
        "bob~", "bonk~", "choice", "fiddle~", "loop~", "lrshift~", "pique", "sigmund~",
    };

    if (std::find(CompiledObjects.begin(), CompiledObjects.end(), pl.Name) !=
        CompiledObjects.end()) {

        fs::path ExtraPath = p->Pd4WebRoot / "pure-data" / "extra" / pl.Name;
        fs::path DestinationPath = p->BuildFolder / "Pd4Web" / "pure-data" / "extra" / pl.Name;
        fs::create_directories(DestinationPath.parent_path()); // garante dirs acima
        fs::copy(ExtraPath, DestinationPath,
                 fs::copy_options::recursive | fs::copy_options::skip_existing);
        pl.Found = true;
        pl.isExtraExternal = true;
        p->ExtraObjects.push_back(pl);
        return;
    }

    std::vector<std::string> Abstractions = {
        "complex-mod~", "hilbert~", "rev1~", "rev2~", "rev3~",
    };

    if (std::find(Abstractions.begin(), Abstractions.end(), pl.Name) != Abstractions.end()) {
        pl.isAbstraction = true;
        fs::path AbsPath = p->Pd4WebRoot / "pure-data" / "extra" / (pl.Name + ".pd");
        if (fs::exists(AbsPath)) {
            auto Abstraction = std::make_shared<Patch>();
            Abstraction->PatchFile = AbsPath;
            Abstraction->PatchFolder = AbsPath.parent_path();
            Abstraction->Pd4WebRoot = p->Pd4WebRoot;
            pl.isAbstraction = true;
            pl.isExternal = false;
            print("\n");
            print("Processing extra Patch '" + Abstraction->PatchFile.filename().string(),
                  Pd4WebLogLevel::PD4WEB_LOG1, p->printLevel + 1);
            processSubpatch(p, Abstraction);

            pl.Found = true;
            return;
        } else {
            print("'" + pl.Name + "' seems to be an extra object, but pd4web couldn't find it",
                  Pd4WebLogLevel::PD4WEB_LOG1, p->printLevel + 1);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isExternalLibObj(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
    if (pl.Found) {
        return;
    }
    std::string libprefix = pl.Lib;

    // object with prefix
    for (Library SupportedLib : m_Libraries) {
        if (SupportedLib.Name == pl.Lib && pl.Lib != "") {
            std::vector<std::string> objects = listObjectsInLibrary(p, pl.Lib);
            for (std::string obj : objects) {
                if (obj == pl.Name) {
                    print("Found external object '" + pl.Name + "' via prefix lib: " + pl.Tokens[4],
                          Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
                    pl.isExternal = true;
                    pl.isAbstraction = false;
                    pl.isLuaExternal = false;
                    pl.Found = true;
                    p->DeclaredLibs.push_back(pl.Lib);
                    p->ExternalObjects.push_back(pl);
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
                print("Found external object '" + pl.Name + "' using lib in declared libs",
                      Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
                pl.Found = true;
                pl.isExternal = true;
                pl.isLuaExternal = false;
                pl.Lib = lib;
                p->ExternalObjects.push_back(pl);
                return;
            }
        }
    }

    isExtraObj(p, pl);
}

// ─────────────────────────────────────
void Pd4Web::isPdObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    PD4WEB_LOGGER();

    // is not pd object, but can be a number or special objects
    if (isNumber(pl.Name)) {
        pl.Found = true;
        return;
    }

    if (pl.Name[0] == '/') {
        pl.Found = true;
        return;
    }

    if (pl.Name.rfind("\\$", 0) == 0) {
        pl.Found = true;
        return;
    }

    // check for midi and audio objects
    isMidiObj(Patch, pl);

    bool isPdObj = std::find(m_PdObjects.begin(), m_PdObjects.end(), pl.Name) != m_PdObjects.end();
    if (!isPdObj || !pl.Lib.empty()) {
        pl.isExternal = true;
        pl.Found = false;
        pl.isLuaExternal = false;
        return;
    } else {
        pl.isExternal = false;
        pl.Found = true;
    }
}

// ─────────────────────────────────────
void Pd4Web::isMidiObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    PD4WEB_LOGGER();
    std::vector<std::string> midiIn = {"notein", "cltin",          "bendin",
                                       "pgmin",  "touchin",        "polytouchin",
                                       "midiin", "midirealtimein", "sysexin"};

    std::vector<std::string> midiOut = {"noteout",  "cltout",       "bendout", "pgmout",
                                        "touchout", "polytouchout", "midiout"};

    if (std::find(midiIn.begin(), midiIn.end(), pl.Name) != midiIn.end()) {
        Patch->Midi = true;
        print("Activate Midi In Support", Pd4WebLogLevel::PD4WEB_LOG2, Patch->printLevel + 1);
    } else if (std::find(midiOut.begin(), midiOut.end(), pl.Name) != midiOut.end()) {
        Patch->Midi = true;
        print("Activate Midi Out Support", Pd4WebLogLevel::PD4WEB_LOG2, Patch->printLevel + 1);
    } else {
        Patch->Midi = false;
    }
}

// ─────────────────────────────────────
std::string Pd4Web::getObjLib(std::string &objToken) {
    PD4WEB_LOGGER();
    if (objToken == "/" && objToken.size() == 1) {
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
    PD4WEB_LOGGER();

    std::string Obj = objToken;
    Obj.erase(std::remove(Obj.begin(), Obj.end(), ','), Obj.end());
    Obj.erase(std::remove(Obj.begin(), Obj.end(), ';'), Obj.end());

    if (Obj == "/" || Obj == "/~") {
        return Obj;
    }

    size_t slashPos = Obj.find_last_of('/');
    if (slashPos != std::string::npos) {
        Obj = Obj.substr(slashPos + 1);
    }
    print("Processing object '" + Obj + "'", Pd4WebLogLevel::PD4WEB_LOG2);
    return Obj;
}

// ─────────────────────────────────────
bool Pd4Web::processCanvasAtoms(std::shared_ptr<Patch> &p, PatchLine &pl) {
    if (p->CanvasLevel == 1) {
        std::string xpix = pl.Tokens[2];
        std::string ypix = pl.Tokens[3];
        std::vector<std::string> updatedTokens = {"#X", "obj", xpix, ypix};
        switch (pl.Type) {
        case PatchLine::FLOATATOM:
            updatedTokens.push_back("floatatom");
            break;
        case PatchLine::SYMBOLATOM:
            updatedTokens.push_back("symbolatom");
            break;
        case PatchLine::LISTATOM:
            updatedTokens.push_back("listatom");
            break;
        default:
            print("Wrong assumption, please report", Pd4WebLogLevel::PD4WEB_ERROR);
        }

        p->PdLua = true;
        p->LuaGuiObjects = true;
        print("Replacing " + updatedTokens[4] + " by pdlua " + updatedTokens[4],
              Pd4WebLogLevel::PD4WEB_LOG2);
        for (int i = 4; i < pl.Tokens.size(); i++) {
            updatedTokens.push_back(pl.Tokens[i]);
        }
        pl.Tokens = updatedTokens;
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processObjAudioInOut(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
    int length = pl.Tokens.size();
    std::string Obj = getObjName(pl.Tokens[4]);
    std::string Lib = getObjLib(pl.Tokens[4]);

    if (Obj == "adc~") {
        unsigned int input = 0;
        if (length > 5) {
            for (size_t i = 5; i < pl.Tokens.size(); ++i) {
                std::string token = pl.Tokens[i];
                if (isNumber(token)) {
                    input = std::stoi(token);
                }
            }
        }

        if (!input) {
            input = 1;
        }
        p->Input = input;
        print("Number of input is " + std::to_string(input), Pd4WebLogLevel::PD4WEB_LOG2,
              p->printLevel + 1);
    } else if (Obj == "dac~") {
        unsigned int output = 0;

        if (length > 5) {
            for (size_t i = 5; i < pl.Tokens.size(); ++i) {
                std::string token = pl.Tokens[i];
                if (isNumber(token)) {
                    output = std::stoi(token);
                }
            }
        }

        if (!output) {
            output = 1;
        }
        p->Output = output;
        print("Number of output is " + std::to_string(output), Pd4WebLogLevel::PD4WEB_LOG2,
              p->printLevel + 1);
        return true;
    }
    return false;
}

// ─────────────────────────────────────
static bool isCloneSubPatchToken(const std::unordered_set<std::string> &args,
                                 const std::string &token) {
    PD4WEB_LOGGER();
    if (token.empty()) {
        return false;
    }

    std::string t = token;
    // only strip trailing ',' or ';'
    if (t.back() == ',' || t.back() == ';') {
        t.pop_back();
    }

    if (args.find(t) != args.end()) {
        return false;
    }
    if (t[0] == '$' || t[0] == '\\') {
        return false;
    }
    if (std::all_of(t.begin(), t.end(), ::isdigit)) {
        return false;
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processObjClone(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();

    std::unordered_set<std::string> args = {"#X", "obj", "clone", "-do", "-di", "-x", "-s", "f"};

    std::vector<std::string> filtered;
    std::copy_if(pl.Tokens.begin(), pl.Tokens.end(), std::back_inserter(filtered),
                 [&](const std::string &token) { return isCloneSubPatchToken(args, token); });

    if (filtered.size() != 1) {
        for (auto filteredToken : filtered) {
            print(filteredToken + " ", Pd4WebLogLevel::PD4WEB_ERROR);
        }
        print("Filtered should return just 1 result, please report", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    } else {
        std::string abs = filtered[0];
        abs.erase(
            std::remove_if(abs.begin(), abs.end(), [](char c) { return c == ';' || c == ','; }),
            abs.end());
        pl.CloneAbs = abs;
        fs::path AbsPath = getAbsPath(p, pl);
        if (!fs::exists(AbsPath)) {
            return false;
        }
        auto Abstraction = std::make_shared<Patch>();
        Abstraction->PatchFile = AbsPath;
        Abstraction->PatchFolder = AbsPath.parent_path();
        pl.isAbstraction = true;
        pl.isExternal = false;
        print("\n");
        print("Processing clone SubPatch '" + abs + ".pd'", Pd4WebLogLevel::PD4WEB_LOG1,
              p->printLevel + 1);
        processSubpatch(p, Abstraction);
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processObjClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
    std::string Obj = getObjName(pl.Tokens[4]);
    std::string Lib = getObjLib(pl.Tokens[4]);
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

    // if not found it set isExternal to true
    isPdObj(p, pl);

    if (pl.isExternal) {
        if (p->PdLua) {
            isLuaObj(p, pl);
        }

        isAbstraction(p, pl);
        isExternalLibObj(p, pl);

        if (!pl.Found) {
            print("Not Found object '" + Obj +
                      "'. If this is an external object check "
                      "https://charlesneimog.github.io/pd4web/patch/externals/",
                  Pd4WebLogLevel::PD4WEB_ERROR, p->printLevel + 1);
            print(pl.Line, Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processDeclareClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    PD4WEB_LOGGER();
    size_t index = 2;
    while (index < pl.Tokens.size()) {
        std::string Token = pl.Tokens[index];
        std::string Lib = pl.Tokens[index + 1];
        if (!Lib.empty() && Lib.back() == ';') {
            Lib.pop_back();
        }
        if (Token == "-lib") {
            if (libIsSupported(Lib)) {
                p->DeclaredLibs.push_back(Lib);
                std::vector<std::string> objectsInLibrary = listObjectsInLibrary(p, Lib);
                p->ValidObjectNames.insert(p->ValidObjectNames.end(), objectsInLibrary.begin(),
                                           objectsInLibrary.end());
                print("Found declare library " + Lib, Pd4WebLogLevel::PD4WEB_LOG2,
                      p->printLevel + 1);
                (void)downloadSupportedLib(Lib);
                if (Lib == "pdlua") {
                    p->PdLua = true;
                }
            } else {
                print("Declared lib " + Lib + " not supported", Pd4WebLogLevel::PD4WEB_LOG2,
                      p->printLevel + 1);
            }

            // check if library is cloned

        } else if (Token == "-path") {
            p->DeclaredPaths.push_back(Lib);
            if (libIsSupported(Lib)) {
                p->DeclaredLibs.push_back(Lib);
                print("Found declare library " + Lib, Pd4WebLogLevel::PD4WEB_LOG2,
                      p->printLevel + 1);
            }
            print("Found declare path " + Lib, Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
        }

        index += 2;
    }
    return true;
}

// ─────────────────────────────────────
void Pd4Web::updatePatchFile(std::shared_ptr<Patch> &p, bool mainPatch) {
    PD4WEB_LOGGER();

    auto strip_lib_preserve_semicolon = [](std::string s) -> std::string {
        bool had_semicolon = !s.empty() && s.back() == ';';
        if (had_semicolon) {
            s.pop_back();
        }
        size_t pos = s.rfind('/');
        if (pos != std::string::npos) {
            s = s.substr(pos + 1);
        }
        if (had_semicolon) {
            s.push_back(';');
        }
        return s;
    };

    auto strip_lib = [](std::string s) -> std::string {
        size_t pos = s.rfind('/');
        if (pos != std::string::npos) {
            s = s.substr(pos + 1);
        }
        return s;
    };

    auto is_number = [](const std::string &s) {
        if (s.empty()) {
            return false;
        }
        size_t start = (s[0] == '-' ? 1 : 0);
        if (start == s.size()) {
            return false;
        }
        return std::all_of(s.begin() + start, s.end(),
                           [](unsigned char c) { return std::isdigit(c); });
    };

    std::string editPatch;

    for (auto &pl : p->PatchLines) {
        // 1) Remover prefixo de lib em '#X obj' quando houver '/' no token do objeto,
        //    independente de pl.isExternal/pl.isAbstraction. Exceção: 'clone' (tratado abaixo).
        if (pl.Type == PatchLine::OBJ && pl.Tokens.size() > 4) {
            auto &objNameTok = pl.Tokens[4];
            if (pl.Name == "clone") {
                // 2) Tratamento especial para clone: remover prefixos dos tokens que
                // representam
                //    o nome da abstração, preservando ';' e ignorando flags e números.
                print("Editing clone object from '" + objNameTok + "'");
                static const std::unordered_set<std::string> reserved{"#X",  "obj", "clone", "-do",
                                                                      "-di", "-x",  "-s",    "f"};
                for (std::size_t i = 0; i < pl.Tokens.size(); ++i) {
                    auto &tok = pl.Tokens[i];
                    if (reserved.count(tok) == 0 && !is_number(tok)) {
                        std::string before = tok;
                        tok = strip_lib_preserve_semicolon(tok);
                        if (before != tok) {
                            print("  clone arg: '" + before + "' -> '" + tok + "'",
                                  Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
                        }
                    }
                }

            } else {
                // 3) Para qualquer outro objeto, se houver prefixo 'lib/', remova.
                if (objNameTok.find('/') != std::string::npos && pl.Name != "/" &&
                    pl.Name != "/~") {
                    std::string oldTok = objNameTok;
                    objNameTok = strip_lib_preserve_semicolon(objNameTok);
                    print("Editing external/abstraction object: '" + oldTok + "' -> '" +
                              objNameTok + "'",
                          Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
                }
            }
        }

        // 4) Substituição de objetos de GUI no canvas raiz do patch principal
        if (pl.Type == PatchLine::OBJ && pl.Tokens.size() > 4) {
            static const std::unordered_set<std::string> guiObjs{
                "vsl", "hsl", "vradio", "hradio", "tgl", "nbx", "bng", "keyboard", "vu"};
            std::string baseName = strip_lib(pl.Name);
            if (guiObjs.count(baseName) && mainPatch && p->CanvasLevel == 1) {
                const auto &oldTok = pl.Tokens[4];
                bool hadSemi = !oldTok.empty() && oldTok.back() == ';';
                pl.Tokens[4] = "l." + baseName + (hadSemi ? ";" : "");
                p->PdLua = true;
                p->LuaGuiObjects = true;
                print("Replacing Gui Object '" + baseName + "' with 'l." + baseName + "'",
                      Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
            }
        }

        for (const auto &token : pl.Tokens) {
            editPatch += token + " ";
        }
        editPatch += "\n";
    }

    if (mainPatch) {
        print("\n");
        print("Saving " + p->PatchFile.filename().string() + " as index.pd",
              Pd4WebLogLevel::PD4WEB_LOG1);
        writeFile((p->BuildFolder / "WebPatch" / "index.pd").string(), editPatch);
    } else {
        print("Creating modified version of " + p->PatchFile.filename().string(),
              Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
        fs::create_directories(p->BuildFolder / ".tmp");
        writeFile((p->BuildFolder / ".tmp" / p->PatchFile.filename()).string(), editPatch);
    }
}

// ─────────────────────────────────────
bool Pd4Web::processSubpatch(std::shared_ptr<Patch> &f, std::shared_ptr<Patch> &p) {
    PD4WEB_LOGGER();

    for (auto &c : f->Childs) {
        if (c->PatchFile == p->PatchFile) {
            return true;
        }
    }

    p->Father = f;
    p->PatchFolder = f->PatchFolder;
    p->DeclaredPaths = f->DeclaredPaths;
    p->DeclaredLibs = f->DeclaredLibs;
    p->printLevel = p->printLevel + 1;
    p->BuildFolder = f->BuildFolder;
    p->Pd4WebFiles = f->Pd4WebFiles;
    p->Pd4WebRoot = f->Pd4WebRoot;

    f->Childs.push_back(p);
    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }
    getSupportedLibraries(p);

    if (m_PdObjects.empty()) {
        print("Pd Objects should already be listed here, this is an internal error, please "
              "report",
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    unsigned int i = 0;
    for (PatchLine &Line : p->PatchLines) {
        ok = processLine(p, Line);
        if (!ok) {
            print("Failed to process Abstraction line " + std::to_string(i) + ". " +
                      p->PatchFile.string(),
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        i++;
    }

    // update father declare libs and paths
    for (auto lib : p->DeclaredLibs) {
        f->DeclaredLibs.push_back(lib);
    }
    for (auto lib : p->DeclaredPaths) {
        f->DeclaredPaths.push_back(lib);
    }
    for (auto &pl : p->PatchLines) {
        if (pl.isExternal || pl.isAbstraction) {
            f->ExternalObjects.push_back(pl);
        }

        if (pl.isExtraExternal) {
            f->ExtraObjects.push_back(pl);
        }
    }

    updatePatchFile(p);
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::isUniqueObjFromLibrary(std::shared_ptr<Patch> &p, std::string &Obj) {
    PD4WEB_LOGGER();
    for (Library Lib : m_Libraries) {
        if (Lib.Name == Obj) {
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
void Pd4Web::updateTemplate(std::shared_ptr<Patch> &p) {

    if (p->TemplateId != 0) {
        fs::path templatePath = p->Pd4WebFiles / "Templates" / std::to_string(p->TemplateId);
        if (fs::exists(templatePath) && fs::is_directory(templatePath)) {
            for (const auto &entry : fs::directory_iterator(templatePath)) {
                if (entry.is_regular_file()) {
                    fs::path target = p->BuildFolder / "WebPatch" / entry.path().filename();
                    fs::copy(entry.path(), target, fs::copy_options::skip_existing);
                }
            }
        } else {
            print("Template folder not found: " + templatePath.string() + ", please report!",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return;
        }
    } else {
        fs::copy(p->Pd4WebFiles / "index.html", p->BuildFolder / "WebPatch" / "index.html",
                 fs::copy_options::skip_existing);
    }
}

// ─────────────────────────────────────
bool Pd4Web::compilePatch() {
    PD4WEB_LOGGER();
    if (!m_Init) {
        print("Pd4Web init failed", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    m_Error = false;

    validateArgs();

    auto p = std::make_shared<Patch>();
    p->PatchFile = fs::path(m_PatchFile);
    p->PatchFolder = p->PatchFile.parent_path();
    p->Zoom = m_PatchZoom;
    p->PdVersion = m_PdVersion;
    p->ProcessedSubpatches.clear();
    p->MemorySize = m_Memory;
    p->RenderGui = m_RenderGui;
    p->TemplateId = m_TemplateId;

    if (m_Pd4WebFiles == "") {
        print("m_Pd4WebFiles not set", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    p->Pd4WebFiles = m_Pd4WebFiles;
    p->PdLua = true;
    p->ProjectName = p->PatchFile.parent_path().filename().string();
    p->Pd4WebRoot = m_Pd4WebRoot;
    print("Project name is: '" + p->ProjectName + "'", Pd4WebLogLevel::PD4WEB_LOG1);

    if (m_BuildFolder != "") {
        p->BuildFolder = fs::path(m_BuildFolder);
    } else {
        p->BuildFolder = p->PatchFolder;
    }

    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    print("Getting PureData Internal Objects", Pd4WebLogLevel::PD4WEB_VERBOSE, p->printLevel);
    getSupportedLibraries(p);

    if (m_PdObjects.empty()) {
        print("Listing PureData Objects", Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
        m_PdObjects = listObjectsInLibrary(p, "pure-data/src");
        m_PdObjects.push_back("list");
    }
    p->ValidObjectNames = m_PdObjects;

    print("\n");
    print("Processing Patch '" + p->ProjectName + ".pd'", Pd4WebLogLevel::PD4WEB_LOG1,
          p->printLevel);
    unsigned int i = 1;
    for (PatchLine &PatchLine : p->PatchLines) {
        ok = processLine(p, PatchLine);
        if (!ok) {
            print("Failed to process Patch of line " + std::to_string(i) + ": " +
                      p->PatchFile.string(),
                  Pd4WebLogLevel::PD4WEB_ERROR, p->printLevel + 1);
            return false;
        }

        i++;
    }

    fs::create_directory(p->BuildFolder / "WebPatch");
    fs::create_directory(p->BuildFolder / "Pd4Web");
    fs::create_directory(p->BuildFolder / "Pd4Web" / "Externals");
    fs::create_directory(p->BuildFolder / "Pd4Web" / "Gui");

    getSupportedLibraries(p);
    updatePatchFile(p, true);
    configureProjectToCompile(p);

    if (m_Error) {
        print("Errors found, fix them before compiling!", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    print("\n");
    print("Start building Patch", Pd4WebLogLevel::PD4WEB_LOG1);
    buildPatch(p);
    createAppManifest(p);
    updateTemplate(p);

    if (!m_Error) {
        print("Finished", Pd4WebLogLevel::PD4WEB_LOG1);
    }

    return true;
}
