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
void Pd4Web::isExternalLibObj(std::shared_ptr<Patch> &Patch, PatchLine &pl) {
    auto pos = pl.Name.find('/'); // find slash
    if (pos == std::string::npos) {
        return;
    }

    std::string lib = pl.Name.substr(0, pos);
    for (Library SupportedLib : m_Libraries) {
        if (SupportedLib.Name == lib) {
            std::vector<std::string> objects = listObjectsInLibrary(lib);
            for (std::string obj : objects) {
                if (obj == pl.Name) {
                    pl.Found = true;
                }
            }
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
        print("Activate Midi Support", Pd4WebColor::GREEN);
    } else if (std::find(midiOut.begin(), midiOut.end(), pl.Name) != midiOut.end()) {
        Patch->Midi = true;
        print("Activate Midi Support", Pd4WebColor::GREEN);
    } else {
        Patch->Midi = false;
    }
}

// ─────────────────────────────────────
bool Pd4Web::processObjClass(std::shared_ptr<Patch> &p, PatchLine &pl) {
    LOG(__FUNCTION__);
    std::string Obj = pl.OriginalTokens[4];
    int length = pl.OriginalTokens.size();
    Obj.erase(std::remove(Obj.begin(), Obj.end(), ','), Obj.end());
    Obj.erase(std::remove(Obj.begin(), Obj.end(), ';'), Obj.end());
    pl.Name = Obj;

    if (Obj == "declare") {
        return true;
    }

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
        print("Number of input is " + std::to_string(input), Pd4WebColor::GREEN);
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
        print("Number of output is " + std::to_string(output), Pd4WebColor::GREEN);
        return true;
    }

    if (Obj == "clone") {
        std::unordered_set<std::string> args = {"#X", "obj", "clone", "-do", "-di", "-x", "-s"};
        std::vector<std::string> filtered;
        std::copy_if(pl.OriginalTokens.begin(), pl.OriginalTokens.end(),
                     std::back_inserter(filtered), [&args](const std::string &token) {
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
            p->Childs.push_back(Abstraction);
            print("Processing SubPatch " + abs, Pd4WebColor::GREEN);
            processSubpatch(Abstraction);
        }

        // Get Abs
        // Check for abs inside Current Path
        // Check for abs inside Library
    } else if (Obj == "pdlua") {
        p->PdLua = true;
        return true;
    }

    isPdObj(p, pl);

    if (pl.isExternal) {
        if (p->PdLua) {
            isLuaObj(p, pl);
        }

        isExternalLibObj(p, pl);

        if (!pl.Found) {
            print("Object '" + pl.Name + "' not found", Pd4WebColor::RED);
            return false;
        }
    }

    isMidiObj(p, pl);

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
            p->DeclaredLibs.push_back(Lib);
            std::vector<std::string> objectsInLibrary = listObjectsInLibrary(Lib);
            p->ValidObjectNames.insert(p->ValidObjectNames.end(), objectsInLibrary.begin(),
                                       objectsInLibrary.end());
            print("Found declare library " + Lib, Pd4WebColor::GREEN);
        } else if (Token == "-path") {
            p->DeclaredPaths.push_back(Lib);
            print("Found declare path " + Lib, Pd4WebColor::GREEN);
        }
        index += 2;
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processSubpatch(std::shared_ptr<Patch> &p) {
    LOG(__FUNCTION__);
    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    getSupportedLibraries(p);

    if (m_PdObjects.empty()) {
        LOG("Pd Objects should already be listed here");
    }

    unsigned int i = 0;
    for (PatchLine Line : p->PatchLines) {
        ok = processLine(p, Line);
        if (!ok) {
            LOG("Failed to process subpatch line" + std::to_string(i))
            return false;
        }
        i++;
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
bool Pd4Web::configureExternalsObjects(std::shared_ptr<Patch> &p) {
    LOG(__FUNCTION__);
    for (PatchLine &Obj : p->ExternalObjects) {
        if (isUniqueObjFromLibrary(p, Obj.Name)) {
            printf("%s is unique library\n", Obj.Name.c_str());
        }
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::processPatch() {
    LOG(__FUNCTION__);

    auto p = std::make_shared<Patch>();
    p->Path = fs::path(m_PatchFile);
    p->Root = p->Path.parent_path();

    bool ok = openPatch(p);
    if (!ok) {
        return false;
    }

    LOG("Getting PureData Internal Objects");
    getSupportedLibraries(p);
    if (m_PdObjects.empty()) {
        print("Listing PureData Objects", Pd4WebColor::GREEN);
        m_PdObjects = listObjectsInLibrary("pure-data/src");
        m_PdObjects.push_back("list");
    }
    p->ValidObjectNames = m_PdObjects;

    unsigned int i = 1;
    for (PatchLine &PatchLine : p->PatchLines) {
        ok = processLine(p, PatchLine);
        if (!ok) {
            print("Failed to process line " + std::to_string(i), Pd4WebColor::RED);
            return false;
        }
        i++;
    }

    ok = configureExternalsObjects(p);
    if (!ok) {
        print("Error configuring externals", Pd4WebColor::RED);
        return false;
    }

    return true;
}
