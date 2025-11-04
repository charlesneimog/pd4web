#pragma once

/*
    TODO: Update template files
    TODO: Implement support to templates
    TODO: Implement lua objects
*/

#include <functional>
#include <memory>
#include <string>
#include <filesystem>
#include <sys/stat.h>

#include <httplib.h>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <fkYAML/node.hpp>

#if defined(_WIN32)
#include <windows.h>
#endif

extern "C" {

#include <git2.h>
#include <tree_sitter/api.h>

const TSLanguage *tree_sitter_cpp(void);
const TSLanguage *tree_sitter_c(void);
}

#define PUREDATA_VERSION "0.56-0"
#define EMSDK_VERSION "4.0.10"

using json = nlohmann::json;
using YamlNode = ::fkyaml::v0_4_2::basic_node<>;
namespace fs = std::filesystem;

// ──────────────────────────────────────────
class Pd4WebLogger {
  public:
    Pd4WebLogger(const char *name) {
        m_Name = name;
        std::cout << "[Enter] " << name << std::endl;
    }
    ~Pd4WebLogger() {
        std::cout << "[Exit] " << m_Name << std::endl;
    }

  private:
    std::string m_Name;
};

#define PD4WEB_LOGGER_ENABLED
#if defined(PD4WEB_LOGGER_ENABLED)
#define PD4WEB_LOGGER() Pd4WebLogger pd4web_logger_##__LINE__(__FUNCTION__)
#else
#define PD4WEB_LOGGER()                                                                            \
    do {                                                                                           \
    } while (0)
#endif

// ──────────────────────────────────────────
enum class Pd4WebLogLevel {
    PD4WEB_WARNING = 0,
    PD4WEB_ERROR,
    PD4WEB_LOG1,
    PD4WEB_LOG2,
    PD4WEB_VERBOSE
};

// ──────────────────────────────────────────
struct PatchLine {
    enum PatchTokenType {
        DECLARE = 0,
        OBJ,
        CONNECTION,
        CANVAS,
        RESTORE,
        MSG,
        ARRAY,
        COORDS,
        FLOATATOM,
        TEXT,
        INVALID
    };
    PatchTokenType Type = INVALID;
    std::string OriginalLine;
    std::vector<std::string> OriginalTokens;
    std::vector<std::string> ModifiedTokens;

    bool isLuaExternal = false;
    bool isExternal = false;
    bool isAbstraction = false;
    bool Found = false;

    std::string Name;
    std::string Lib;
    std::string CloneAbs;
};

// ──────────────────────────────────────────
struct Patch {
    fs::path PatchFile;
    fs::path PatchFolder;
    fs::path WebPatchFolder;
    fs::path Pd4WebFiles;
    fs::path Pd4WebRoot;
    fs::path mainRoot;

    std::string ProjectName;

    int MemorySize = 64;
    int Zoom = 1;
    std::vector<PatchLine> PatchLines;
    std::vector<PatchLine> ExternalPatchLines;
    std::string PdVersion = PUREDATA_VERSION;

    json ExternalObjectsJson;

    // Objects
    std::vector<PatchLine> ExternalObjects;
    std::vector<std::string> DeclaredPaths;
    std::vector<std::string> DeclaredLibs;
    std::vector<std::string> ValidObjectNames;
    std::vector<std::string> ValidLuaObjects;
    std::vector<std::string> ProcessedSubpatches;

    std::vector<fs::path> PdLuaFolderSearch;

    bool Midi;
    bool PdLua;
    bool LuaGuiObjects;
    unsigned Input;
    unsigned Output;
    unsigned Sr = 48000;

    std::shared_ptr<Patch> Father;
    std::vector<std::shared_ptr<Patch>> Childs;
    int printLevel = 1; // just to better debug info

    int CanvasLevel = 0;
};

// ──────────────────────────────────────────
struct Library {
    std::string Name;
    std::string Source;
    std::string Developer;
    std::string Repository;
    std::string Version;
    std::string Url;
};

// ──────────────────────────────────────────
using Libraries = std::vector<Library>;

// ──────────────────────────────────────────
class Pd4Web {
  public:
    Pd4Web(std::string pathHome);
    void parseArgs(int argc, char *argv[]);
    bool init();
    bool compilePatch();

    void setPatchFile(std::string file) {
        m_PatchFile = file;
    };
    void setPd4WebFilesFolder(std::string path) {
        m_Pd4WebFiles = path;
    };
    void setInitialMemory(int memory) {
        m_Memory = memory;
    };
    void setPatchZoom(int zoom) {
        m_PatchZoom = zoom;
    };
    void setOutputFolder(std::string folder) {
        m_OutputFolder = folder;
    };
    void setTemplateId(int id) {
        m_TemplateId = id;
    };
    void setDebugMode(bool debug) {
        m_Debug = debug;
    };
    void setDevDebugMode(bool debug) {
        m_DevDebug = debug;
    };
    void setFailFast(bool failfast) {
        m_FailFast = failfast;
    };
    void setCleanBuild(bool clean) {
        m_CleanBuild = clean;
    }

    void disableGuiRender() {
        m_RenderGui = false;
    };

    void setPrintCallback(std::function<void(const std::string &, Pd4WebLogLevel, int)> cb) {
        m_PrintCallback = cb;
    }

    void serverPatch(bool server = false);

  private:
    bool m_Init;
    bool m_Error;

    std::string m_Pd4WebRoot;   // TODO: is fs::path
    std::string m_OutputFolder; // TODO: is fs::path
    fs::path m_Pd4WebFiles;

    std::string m_PatchFile;     // TODO: is fs::path
    std::string m_LibrariesPath; // TODO: is fs::path
    int m_TemplateId;
    bool m_BypassUnsuported;
    bool m_Verbose;
    std::string m_PdVersion;
    std::string m_EmsdkVersion;
    bool m_Debug = false;
    bool m_DevDebug = false;
    bool m_FailFast = false;
    bool m_CleanBuild = false;
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

    std::function<void(const std::string &, Pd4WebLogLevel, int)> m_PrintCallback;
    bool m_inArray = false;

    // Paths
    bool initPaths();
    std::string getEmsdkPath();
    bool checkAllPaths();
    bool getNinja();
    bool getCmakeBinary();
    bool getNode();

    std::string m_Cmake;
    std::string m_EmsdkInstaller;
    std::string m_Emcmake;
    std::string m_Emcc;
    std::string m_Emconfigure;
    std::string m_Emmake;
    std::string m_Ninja;
    std::string m_Clang;
    std::string m_NodeJs;
    std::string m_PythonWindows;

    // httplib::SSLClient m_GithubCli;

    // Git
    bool gitRepoExists(const std::string &path);
    bool gitClone(std::string git, std::string path, std::string tag);
    bool gitPull(std::string git, std::string path);
    bool gitCheckout(std::string git, std::string path, std::string tag);
    bool isFileFromGitSubmodule(const fs::path &repoRoot, const fs::path &filePath);

    // Cmd
    bool cmdInstallEmsdk();

    // Patch
    bool openPatch(std::shared_ptr<Patch> &Patch);
    bool processLine(std::shared_ptr<Patch> &p, PatchLine &pl);
    bool processSubpatch(std::shared_ptr<Patch> &f, std::shared_ptr<Patch> &p);

    fs::path getAbsPath(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    std::string getObjName(std::string &ObjToken);
    std::string getObjLib(std::string &ObjToken);

    bool processDeclareClass(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    bool processObjClass(std::shared_ptr<Patch> &Patch, PatchLine &pl);

    void isPdObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isLuaObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isExternalLibObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isAbstraction(std::shared_ptr<Patch> &Patch, PatchLine &pl);

    void isMidiObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isCloneObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isFloatObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isDollarObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);
    void isExtraObj(std::shared_ptr<Patch> &Patch, PatchLine &pl);

    void updatePatchFile(std::shared_ptr<Patch> &p, bool mainPatch = false);

    // process
    bool processObjClone(std::shared_ptr<Patch> &p, PatchLine &pl);
    bool processObjAudioInOut(std::shared_ptr<Patch> &p, PatchLine &pl);

    // bool configureExternalsObjects(std::shared_ptr<Patch> &Patch);
    bool isUniqueObjFromLibrary(std::shared_ptr<Patch> &p, std::string &Obj);

    // Libraries
    TSParser *m_cppParser;
    TSParser *m_cParser;
    Libraries m_Libraries;
    bool libIsSupported(std::string libName);
    bool downloadSupportedLib(std::string libName);

    bool getSupportedLibraries(std::shared_ptr<Patch> &Patch);
    bool libsDownload(YamlNode node);
    std::vector<std::string> listObjectsInLibrary(std::shared_ptr<Patch> &p, std::string Lib);
    std::vector<std::string> listAbstractionsInLibrary(std::shared_ptr<Patch> &p, std::string Lib);
    bool findSetupFunction(std::string objName, std::string Lib);

    void processCallExpression(std::string &content, TSNode node,
                               std::vector<std::string> &objectNames,
                               std::vector<std::string> &setupNames,
                               std::vector<std::string> &setupSignatures);
    void treesitterCheckForSetupFunction(std::string &content, TSNode node,
                                         std::vector<std::string> &objectNames,
                                         std::vector<std::string> &setupNames,
                                         std::vector<std::string> &setupSignatures);

    std::vector<fs::path> findLuaObjects(std::shared_ptr<Patch> &Patch, fs::path Folder,
                                         PatchLine &pl);

    YamlNode m_SourcesNode;
    YamlNode m_LibrariesNode;
    std::vector<std::string> m_DeclaredLibraries;
    std::vector<std::string> m_DeclaredPaths;

    // Builder
    std::string m_MainCmake;
    void configureProjectToCompile(std::shared_ptr<Patch> &p);
    void createConfigFile(std::shared_ptr<Patch> &p);
    void copyCmakeLibFiles(std::shared_ptr<Patch> &p, std::string Lib);
    void createMainCmake(std::shared_ptr<Patch> &p);
    void createExternalsCppFile(std::shared_ptr<Patch> &p);
    void copySources(std::shared_ptr<Patch> &p);
    void buildPatch(std::shared_ptr<Patch> &p);
    void createAppManifest(std::shared_ptr<Patch> &p);
    void copyExtraSources(std::shared_ptr<Patch> &p, fs::path buildDir);

    // Utils
    std::string getCertFile();
    int execProcess(const std::string &command, std::vector<std::string> &args);
    std::string formatLibUrl(const std::string &format, const std::string &arg1,
                             const std::string &arg2);
    bool isNumber(const std::string &s);
    void print(std::string msg, enum Pd4WebLogLevel color = Pd4WebLogLevel::PD4WEB_LOG2,
               int level = 1);

    std::string readFile(const std::string &path);
    void writeFile(const std::string &path, const std::string &content);
    void replaceAll(std::string &str, const std::string &from, const std::string &to);
};
