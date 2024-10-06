#include <m_pd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <string>
#include <thread>

#include <m_imp.h>

static t_class *pd4web_class;

static std::atomic<bool> Pd4WebRunning(false);
static std::atomic<bool> Pd4WebKill(false);

// ─────────────────────────────────────
class Pd4Web {
  public:
    t_object Obj;
    bool pythonInstalled;
    bool pipInstalled;
    bool pd4webInstalled;
    std::string Pd4WebPath;
    std::string Pd4WebExe;
    bool Pd4WebIsReady;

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
    post("%s --help", x->Pd4WebPath.c_str());
    std::string cmd = x->Pd4WebPath + " --help";
    result = std::system(cmd.c_str());
    return (result == 0);
}

// ─────────────────────────────────────
static void RunCmd(Pd4Web *x, std::string cmd, bool browser) {
    if (browser) {
        std::thread([x, cmd]() {

        }).detach();
    } else {
        std::thread([x, cmd]() {
            post("[pd4web] Running...");
            Pd4WebRunning.store(true);
            std::array<char, 256> Buf;
            std::string Result;
            FILE *Pipe = popen(cmd.c_str(), "r");
            if (!Pipe) {
                pd_error(nullptr, "[pd4web] popen failed");
                Pd4WebRunning.store(false); // Ensure the flag is reset on failure
                return;
            }

            while (!Pd4WebKill.load() && fgets(Buf.data(), Buf.size(), Pipe) != nullptr) {
                std::string Line = "[pd4web] ";
                Line += Buf.data();
                // remove \n
                Line.erase(std::remove(Line.begin(), Line.end(), '\n'), Line.end());

                if (Line != "[pd4web] ") {
                    post(Line.c_str());
                }
            }
            post("[pd4web] Done!");
            pclose(Pipe);
            return;
        }).detach();
    }
}

// ─────────────────────────────────────
static void RunBrowser(Pd4Web *x) {
    if (!x->Pd4WebIsReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }

    if (x->patch == "") {
        pd_error(x, "[pd4web] No patch selected");
        return;
    }

    std::string cmd = x->Pd4WebPath;
    cmd += " --run-browser ";
    cmd += x->patch.c_str();
    RunCmd(x, cmd, true);
}

// ─────────────────────────────────────
static void SetConfig(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    if (argv[0].a_type != A_SYMBOL) {
        pd_error(x, "[pd4web] Invalid argument, use [set <config_symbol> <value>]");
        return;
    }
    std::string config = atom_getsymbol(argv)->s_name;
    if (config == "memory") {
        int memory = atom_getintarg(1, argc, argv);
        if (memory != x->memory) {
            x->memory = memory;
            post("[pd4web] Initial memory set to %dMB", x->memory);
        }
        return;
    } else if (config == "verbose") {
        bool verbose = atom_getintarg(1, argc, argv);
        if (verbose != x->verbose) {
            x->verbose = verbose;
            if (x->verbose) {
                post("[pd4web] Verbose set to true");
            } else {
                post("[pd4web] Verbose set to false");
            }
        }
        return;
    } else if (config == "gui") {
        bool gui = atom_getintarg(1, argc, argv);
        if (gui != x->gui) {
            x->gui = gui;
            if (x->gui) {
                post("[pd4web] Gui set to true");
            } else {
                post("[pd4web] Gui set to false");
            }
        }
    } else if (config == "patch") {
        x->patch = atom_getsymbolarg(1, argc, argv)->s_name;
        post("[pd4web] Patch set");
    }
    return;
}

// ─────────────────────────────────────
static void Compile(Pd4Web *x) {
    if (!x->Pd4WebIsReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }

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

    RunCmd(x, cmd, false);

    return;
}

// ─────────────────────────────────────
static void *Pd4WebNew(t_symbol *s, int argc, t_atom *argv) {
    Pd4Web *x = (Pd4Web *)pd_new(pd4web_class);
    x->Out = outlet_new(&x->Obj, &s_anything);
    GetPd4WebExe(x);
    x->Pd4WebPath = pd4web_class->c_externdir->s_name + x->Pd4WebExe;

    std::thread([x]() { x->Pd4WebIsReady = CheckPd4Web(x); }).detach();

    // default variables
    x->verbose = true;
    x->memory = 32;

    return x;
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)Pd4WebNew, 0, sizeof(Pd4Web),
                             CLASS_DEFAULT, A_GIMME, A_NULL);

    class_addmethod(pd4web_class, (t_method)SetConfig, gensym("set"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)RunBrowser, gensym("browser"), A_NULL);
    class_addbang(pd4web_class, (t_method)Compile);
}
