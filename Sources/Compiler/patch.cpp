#include "pd4web.hpp"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

// ─────────────────────────────────────
bool Pd4Web::openPatch(std::shared_ptr<Patch> &p) {
    LOG(__FUNCTION__);
    std::ifstream file(p->Path);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo: " << p->Path << std::endl;
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
    LOG(__FUNCTION__);

    std::vector<std::string> Line;
    Line = pl.OriginalTokens;
    if (Line.size() < 2) {
        return true;
    }

    if (Line[1] == "canvas") {
        LOG("Processing canvas");
        pl.Type = PatchLine::CANVAS;
    } else if (Line[1] == "declare") {
        LOG("Processing declare");
        pl.Type = PatchLine::DECLARE;
        processDeclareClass(p, pl);
    } else if (Line[1] == "obj") {
        LOG("Processing obj");
        pl.Type = PatchLine::OBJ;
        processObjClass(p, pl);
    } else if (Line[1] == "restore") {
        LOG("Processing restore");
        pl.Type = PatchLine::RESTORE;
    } else if (Line[1] == "msg") {
        LOG("Processing msg");
        pl.Type = PatchLine::MSG;
    } else if (Line[1] == "connect") {
        LOG("Processing connect");
        pl.Type = PatchLine::CONNECTION;
    }
    return true;
}

// ─────────────────────────────────────
fs::path Pd4Web::getAbsPath(std::shared_ptr<Patch> &p, std::string Abs) {
    bool found = false;
    std::string fullPath = p->Root.string() + "/" + Abs + ".pd";
    if (fs::exists(fullPath)) {
        found = true;
    }

    if (!found) {
        print("Could not find file " + Abs + ".pd", Pd4WebColor::RED, p->printLevel + 1);
        return "";
    }

    return fullPath;
}

// ─────────────────────────────────────
void Pd4Web::isLuaObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    LOG(__FUNCTION__);
    if (Patch->PdLua) {
        std::vector<fs::path> results;
        std::vector<std::string> subdirs = {"Extras", "Lib"};
        for (const auto &subdir : subdirs) {
            findLuaObjects(Patch, Patch->Root / subdir, pl);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isExternalLibObj(std::shared_ptr<Patch> &p, PatchLine &pl) {
    std::string lib = pl.Lib;

    for (Library SupportedLib : m_Libraries) {
        if (SupportedLib.Name == lib) {
            std::vector<std::string> objects = listObjectsInLibrary(p, lib);
            for (std::string obj : objects) {
                if (obj == pl.Name) {
                    pl.Found = true;
                    pl.isExternal = true;
                    p->UsedLibs.push_back(SupportedLib.Name);
                }
            }
        }
    }

    for (Library SupportedLib : m_Libraries) {
        if (SupportedLib.Name == pl.Name) {
            pl.Found = true;
            pl.isExternal = true;
            p->UsedLibs.push_back(pl.Name);
            std::vector<std::string> objects = listObjectsInLibrary(p, pl.Name);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::isPdObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    bool isPdObj = std::find(m_PdObjects.begin(), m_PdObjects.end(), pl.Name) != m_PdObjects.end();
    if (!isPdObj) {
        pl.isExternal = true;
        pl.Found = false;
    } else {
        pl.isExternal = false;
    }

    // check if pl.Name is a number
    if (isNumber(pl.Name)) {
        pl.isExternal = false;
    }

    if (pl.Name[0] == '\\' && pl.Name[1] == '$') {
        pl.isExternal = false;
    }
}

// ─────────────────────────────────────
void Pd4Web::isMidiObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    LOG(__FUNCTION__);
    std::vector<std::string> midiIn = {"notein", "cltin",          "bendin",
                                       "pgmin",  "touchin",        "polytouchin",
                                       "midiin", "midirealtimein", "sysexin"};

    std::vector<std::string> midiOut = {"noteout",  "cltout",       "bendout", "pgmout",
                                        "touchout", "polytouchout", "midiout"};

    if (std::find(midiIn.begin(), midiIn.end(), pl.Name) != midiIn.end()) {
        Patch->Midi = true;
        print("Activate Midi In Support", Pd4WebColor::GREEN, Patch->printLevel + 1);
    } else if (std::find(midiOut.begin(), midiOut.end(), pl.Name) != midiOut.end()) {
        Patch->Midi = true;
        print("Activate Midi Out Support", Pd4WebColor::GREEN, Patch->printLevel + 1);
    } else {
        Patch->Midi = false;
    }
}

// ─────────────────────────────────────
std::string Pd4Web::getObjLib(std::string &objToken) {
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
        print("Number of input is " + std::to_string(input), Pd4WebColor::GREEN, p->printLevel + 1);
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
        print("Number of output is " + std::to_string(output), Pd4WebColor::GREEN,
              p->printLevel + 1);
        return true;
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::processObjClone(std::shared_ptr<Patch> &p, PatchLine &pl) {
    std::unordered_set<std::string> args = {"#X", "obj", "clone", "-do", "-di", "-x", "-s"};
    std::vector<std::string> filtered;
    std::copy_if(pl.OriginalTokens.begin(), pl.OriginalTokens.end(), std::back_inserter(filtered),
                 [&args](const std::string &token) {
                     auto is_number = [](const std::string &s) {
                         return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
                     };
                     return args.find(token) == args.end() && !is_number(token);
                 });

    if (filtered.size() != 1) {
        LOG("Filtered should return just 1 result, please report");
        return false;
    } else {
        std::string abs = filtered[0];
        abs.erase(
            std::remove_if(abs.begin(), abs.end(), [](char c) { return c == ';' || c == ','; }),
            abs.end());

        fs::path AbsPath = getAbsPath(p, abs);
        auto Abstraction = std::make_shared<Patch>();
        Abstraction->Path = AbsPath;
        Abstraction->Root = AbsPath.parent_path();
        Abstraction->Father = p;
        Abstraction->mainRoot = p->mainRoot;
        p->Childs.push_back(Abstraction);
        pl.isAbstraction = true;

        print("\n");
        print("Processing SubPatch '" + abs + ".pd'", Pd4WebColor::BLUE, p->printLevel + 1);
        Abstraction->printLevel = p->printLevel + 1;
        processSubpatch(Abstraction);
    }
    return true;
    // TODO: Get Abs
    // TODO: Check for abs inside Current Path
    // TODO: Check for abs inside Library
}

// ─────────────────────────────────────
bool Pd4Web::processObjClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    LOG(__FUNCTION__);

    int length = pl.OriginalTokens.size();
    std::string Obj = getObjName(pl.OriginalTokens[4]);
    std::string Lib = getObjLib(pl.OriginalTokens[4]);

    pl.Name = Obj;
    pl.Lib = Lib;

    if (Obj == "declare") {
        return true;
    }

    if (Obj == "adc~" || Obj == "dac~") {
        processObjAudioInOut(p, pl);
    }

    if (Obj == "clone") {
        processObjClone(p, pl);
    } else if (Obj == "pdlua") {
        p->PdLua = true;
        p->UsedLibs.push_back("pdlua");
        return true;
    }

    isMidiObj(p, pl);
    isPdObj(p, pl);

    if (pl.isExternal) {
        if (p->PdLua) {
            isLuaObj(p, pl);
        }
        isExternalLibObj(p, pl);
        if (!pl.Found) {
            print("Object '" + pl.Name + "' not found", Pd4WebColor::RED, p->printLevel + 1);
            return false;
        }
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processDeclareClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    LOG(__FUNCTION__);
    size_t index = 2;
    while (index < pl.OriginalTokens.size()) {
        std::string Token = pl.OriginalTokens[index];
        std::string Lib = pl.OriginalTokens[index + 1];
        if (!Lib.empty() && Lib.back() == ';') {
            Lib.pop_back();
        }
        if (Token == "-lib") {
            p->UsedLibs.push_back(Lib);
            std::vector<std::string> objectsInLibrary = listObjectsInLibrary(p, Lib);
            p->ValidObjectNames.insert(p->ValidObjectNames.end(), objectsInLibrary.begin(),
                                       objectsInLibrary.end());
            print("Found declare library " + Lib, Pd4WebColor::GREEN, p->printLevel + 1);
        } else if (Token == "-path") {
            p->DeclaredPaths.push_back(Lib);
            print("Found declare path " + Lib, Pd4WebColor::GREEN, p->printLevel + 1);
        }
        index += 2;
    }
    return true;
}

// ─────────────────────────────────────
void Pd4Web::removePreffix(std::shared_ptr<Patch> &p, bool mainPatch) {
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

        // save
        for (auto &token : pl.OriginalTokens) {
            editPatch += token + " ";
        }
        editPatch += "\n";
    }

    if (mainPatch) {
        writeFile((p->mainRoot / "WebPatch" / "index.pd").string(), editPatch);
    } else {
        writeFile((p->mainRoot / ".tmp" / p->Path.filename()).string(), editPatch);
    }
}

// ─────────────────────────────────────
bool Pd4Web::processSubpatch(std::shared_ptr<Patch> &p) {
    LOG(__FUNCTION__);

    p->printLevel += 1;

    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    getSupportedLibraries(p);

    if (m_PdObjects.empty()) {
        LOG("Pd Objects should already be listed here");
    }

    unsigned int i = 0;
    for (PatchLine &Line : p->PatchLines) {
        ok = processLine(p, Line);
        if (!ok) {
            LOG("Failed to process subpatch line" + std::to_string(i))
            return false;
        }
        i++;
    }

    // TODO: Review this
    for (auto lib : p->UsedLibs) {
        p->Father->UsedLibs.push_back(lib);
    }

    for (auto &pl : p->PatchLines) {
        if (pl.isExternal) {
            p->Father->ExternalPatchLines.push_back(pl);
        }
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::isUniqueObjFromLibrary(std::shared_ptr<Patch> &p, std::string &Obj) {
    LOG(__FUNCTION__);
    for (Library Lib : m_Libraries) {
        if (Lib.Name == Obj) {
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::processPatch() {
    LOG(__FUNCTION__);

    auto p = std::make_shared<Patch>();
    p->Path = fs::path(m_PatchFile);
    p->Root = p->Path.parent_path();
    p->mainRoot = p->Root;
    p->ProjectName = p->Path.parent_path().filename().string();
    p->Pd4WebRoot = m_Pd4WebRoot;

    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    LOG("Getting PureData Internal Objects");
    getSupportedLibraries(p);
    if (m_PdObjects.empty()) {
        print("Listing PureData Objects", Pd4WebColor::GREEN, p->printLevel + 1);
        m_PdObjects = listObjectsInLibrary(p, "pure-data/src");
        m_PdObjects.push_back("list");
    }
    p->ValidObjectNames = m_PdObjects;

    print("\n", Pd4WebColor::GREEN);
    print("Processing Patch '" + p->ProjectName + ".pd'", Pd4WebColor::BLUE, p->printLevel);
    unsigned int i = 1;
    for (PatchLine &PatchLine : p->PatchLines) {
        ok = processLine(p, PatchLine);
        if (!ok) {
            print("Failed to process line " + std::to_string(i), Pd4WebColor::RED,
                  p->printLevel + 1);
            return false;
        }
        i++;
    }

    getSupportedLibraries(p);
    configureProjectToCompile(p);

    removePreffix(p, true);

    buildPatch(p);

    return true;
}
