#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <sys/stat.h>

#include <cxxopts.hpp>
#include <yaml.hpp>

extern "C" {
#include <git2.h>
#include <tree_sitter/api.h>
const TSLanguage *tree_sitter_cpp();
const TSLanguage *tree_sitter_c();
}

#ifdef PDOBJECT
#include <m_pd.h>
#endif

#define PD_VERSION "0.55-2"
#define EMSDK_VERSION "4.0.10"

using YamlNode = ::fkyaml::v0_4_2::basic_node<>;
namespace fs = std::filesystem;

enum Pd4WebColor {
    YELLOW = 0,
    RED,
    GREEN,
    BLUE,
};

struct PatchLine {
    enum PatchTokenType { DECLARE = 0, OBJ, CONNECTION, CANVAS, RESTORE, MSG, INVALID };
    PatchTokenType Type;
    std::vector<std::string> OriginalTokens;
    std::vector<std::string> ModifiedTokens;

    bool isLuaExternal;
    bool isExternal;
    bool Found;

    std::string Name;
};

struct Patch {
    fs::path Path;
    fs::path Root;

    std::vector<PatchLine> PatchLines;

    // Objects
    std::vector<PatchLine> ExternalObjects;
    std::vector<std::string> DeclaredPaths;
    std::vector<std::string> DeclaredLibs;
    std::vector<std::string> ValidObjectNames;
    std::vector<std::string> ValidLuaObjects;

    std::vector<fs::path> PdLuaFolderSearch;

    bool Midi;
    bool PdLua;
    unsigned Input;
    unsigned Output;

    std::shared_ptr<Patch> Father;
    std::vector<std::shared_ptr<Patch>> Childs;
};

struct Library {
    std::string Name;
    std::string Source;
    std::string Developer;
    std::string Repository;
    std::string Version;
    std::string Url;
};

using Libraries = std::vector<Library>;

class Pd4Web {
  public:
    Pd4Web(std::string pathHome);
    void parseArgs(int argc, char *argv[]);
    bool processPatch();

  private:
    std::string m_Pd4WebRoot;

    std::string m_PatchFile;
    std::string m_LibrariesPath;
    int m_TemplateId;
    bool m_BypassUnsuported;
    bool m_Verbose;
    std::string m_PdVersion;
    std::string m_EmsdkVersion;
    bool m_Debug;
    int m_Memory;

    unsigned m_ChnsOutCount;
    unsigned m_ChnsInCount;

    unsigned m_Fps;
    bool m_RenderGui;
    bool m_AutoTheme;
    float m_PatchZoom;
    bool m_PdLua;
    bool m_UsingMidi;
    std::vector<std::string> m_PdObjects;

    // Paths
    bool initPaths();
    std::string getEmsdkPath();
    bool checkAllPaths();
    std::string m_EmsdkInstaller;
    std::string m_Emcmake;
    std::string m_Emcc;
    std::string m_Emconfigure;
    std::string m_Emmake;

    // Git
    bool gitRepoExists(const std::string &path);
    bool gitClone(std::string git, std::string path, std::string tag);
    bool gitPull(std::string git, std::string path);
    bool gitCheckout(std::string git, std::string path, std::string tag);
    bool isFileFromGitSubmodule(const fs::path &repoRoot, const fs::path &filePath);

    // Cmd
    bool cmdExecute(std::string cmd);
    bool cmdInstallEmsdk();

    // Patch
    bool openPatch(std::shared_ptr<Patch> &Patch);
    bool processLine(std::shared_ptr<Patch> &p, PatchLine &pl);
    bool processSubpatch(std::shared_ptr<Patch> &Patch);
    fs::path getAbsPath(std::shared_ptr<Patch> &Patch, std::string Abs);

    bool processDeclareClass(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    bool processObjClass(std::shared_ptr<Patch> &Patch, PatchLine &pl);

    void isPdObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isLuaObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isExternalLibObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);

    void isMidiObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isCloneObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isFloatObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isDollarObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isExtraObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);

    bool configureExternalsObjects(std::shared_ptr<Patch> &Patch);
    bool isUniqueObjFromLibrary(std::shared_ptr<Patch> &p, std::string &Obj);

    // Libraries
    TSParser *m_cppParser;
    TSParser *m_cParser;
    Libraries m_Libraries;
    bool getSupportedLibraries(std::shared_ptr<Patch> &Patch);
    bool libsDownload(YamlNode node);
    std::vector<std::string> listObjectsInLibrary(std::string Lib);
    bool findSetupFunction(std::string objName, std::string Lib);
    std::vector<fs::path> findLuaObjects(std::shared_ptr<Patch> &Patch, fs::path Folder,
                                         PatchLine &pl);

    YamlNode m_SourcesNode;
    YamlNode m_LibrariesNode;
    std::vector<std::string> m_DeclaredLibraries;
    std::vector<std::string> m_DeclaredPaths;

    // Utils
    std::string formatLibUrl(const std::string &format, const std::string &arg1,
                             const std::string &arg2);
    bool isNumber(const std::string &s);
    void print(std::string msg, enum Pd4WebColor color);
};

#define LOG(msg)                                                                                   \
    // std::cout << "[LOG]  " << __FUNCTION__ << ":" << __LINE__ << " - " << msg << std::endl;

#ifdef PDOBJECT
#else
#define INFO(msg)                                                                                  \
    std::cout << "[INFO] " << __FUNCTION__ << ":" << __LINE__ << " - " << msg << std::endl;
#endif
