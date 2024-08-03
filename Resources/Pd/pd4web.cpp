#include <m_pd.h>
#include <string>

#include <array>
#include <cstdio>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include <m_imp.h>

static t_class *pd4web_class;

// ─────────────────────────────────────
class Pd4Web {
  public:
    t_object Obj;
    bool pythonInstalled;
    bool pipInstalled;
    bool pd4webInstalled;
    std::string Pd4WebPath;
    std::string Pd4WebExe;

    // config
    bool verbose;
    int memory;
    int gui;
    std::string patch;

    t_outlet *Out;
};

// ─────────────────────────────────────
static void GetPd4WebExe(Pd4Web *x) {
    std::string arch = "/pd4web-";

    // OS
#if defined(_WIN32) || defined(_WIN64)
    arch += "w-";
#elif defined(__APPLE__)
    arch += "m-";
#elif defined(__linux__)
    arch += "l-";
#endif

    // Arch
#if defined(__x86_64__) || defined(_M_X64)
    arch += "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    arch += "arm";
#elif defined(__arm__) || defined(_M_ARM)
    arch += "arm";
#endif

    x->Pd4WebExe = arch;
}

// ─────────────────────────────────────
static bool CheckPd4Web(Pd4Web *x) {
#if defined(_WIN32) || defined(_WIN64)

#else
    std::string allow = "chmod +x " + x->Pd4WebPath;
    int result = std::system(allow.c_str());
    if (result != 0) {
        pd_error(x, "[pd4web] chmod +x failed");
        return false;
    }
#endif
    std::string cmd = x->Pd4WebPath + " --help";
    post("cmd: %s", cmd.c_str());
    result = std::system(cmd.c_str());
    return (result == 0);
}
// ─────────────────────────────────────
static void SetConfig(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    if (argv[0].a_type != A_SYMBOL) {
        pd_error(x, "[pd4web] Invalid argument, use [set <config_symbol> <value>]");
        return;
    }
    std::string config = atom_getsymbol(argv)->s_name;
    if (config == "memory") {
        x->memory = atom_getintarg(1, argc, argv);
        post("[pd4web] Initial memory set to %dMB", x->memory);
        return;
    } else if (config == "verbose") {
        x->verbose = atom_getintarg(1, argc, argv);
        if (x->verbose) {
            post("[pd4web] Verbose set to true");
        } else {
            post("[pd4web] Verbose set to false");
        }
        return;
    } else if (config == "gui") {
        x->gui = atom_getintarg(1, argc, argv);
        if (x->gui) {
            post("[pd4web] Gui set to true");
        } else {
            post("[pd4web] Gui set to false");
        }
    } else if (config == "patch") {
        x->patch = atom_getsymbolarg(1, argc, argv)->s_name;
        post("[pd4web] Patch set");
    }
    return;
}

// ─────────────────────────────────────
static std::string Exec(const std::string &cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// ─────────────────────────────────────
std::string RemoveSpecialCharsCodes(const std::string &input) {
    std::regex colorCodeRegex("\033\\[[0-9;]*m");
    std::regex variationSelectorRegex("\uFE0F");
    std::string withoutColorCodes = std::regex_replace(input, colorCodeRegex, "");
    std::string withoutSpecialCodes =
        std::regex_replace(withoutColorCodes, variationSelectorRegex, "");

    return std::regex_replace(input, colorCodeRegex, "");
}

// ─────────────────────────────────────
static void Compile(Pd4Web *x) {
    std::string cmd = x->Pd4WebPath;
    if (x->verbose) {
        cmd += " --verbose ";
    }
    if (x->memory > 0) {
        cmd += " -m " + std::to_string(x->memory);
    }
    if (!x->gui) {
        cmd += " --nogui ";
    }

    if (x->patch != "") {
        cmd += x->patch;
    }

    std::thread([cmd]() {
        try {
            std::string output = Exec(cmd);
            // split by new line and print line by line
            std::size_t pos = 0;
            std::size_t prev = 0;
            while ((pos = output.find("\n", prev)) != std::string::npos) {
                std::string line = "[pd4web] " + output.substr(prev, pos - prev);
                if (line != "[pd4web] ") {
                    post(line.c_str());
                }
                prev = pos + 1;
            }
            post("[pd4web] Done!");
        } catch (const std::exception &e) {
            pd_error(nullptr, "[pd4web] Error: %s", e.what());
        }
    }).detach();

    return;
}

// ─────────────────────────────────────
static void *Pd4WebNew(t_symbol *s, int argc, t_atom *argv) {
    Pd4Web *x = (Pd4Web *)pd_new(pd4web_class);
    x->Out = outlet_new(&x->Obj, &s_anything);
    GetPd4WebExe(x);
    x->Pd4WebPath = pd4web_class->c_externdir->s_name + x->Pd4WebExe;

    int result = CheckPd4Web(x);
    if (!result) {
        pd_error(x, "[pd4web] There is a problem with pd4web, check your system.");
        return nullptr;
    }

    // default variables
    x->verbose = false;
    x->memory = 32;

    return x;
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)Pd4WebNew, 0, sizeof(Pd4Web),
                             CLASS_DEFAULT, A_GIMME, A_NULL);

    class_addmethod(pd4web_class, (t_method)SetConfig, gensym("set"), A_GIMME, A_NULL);
    class_addbang(pd4web_class, (t_method)Compile);
}
