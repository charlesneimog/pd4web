#include <m_pd.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <thread>

#include <m_imp.h>

#include "./cpp-httplib/httplib.h"

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
    bool Pd4WebIsReady;

    // threads
    std::thread::id CompilerThread;
    std::thread::id BrowserThread;

    // Server
    httplib::Server *Server;
    std::string ProjectRoot;

    // config
    bool verbose;
    int memory;
    int gui;
    std::string patch;

    t_outlet *Out;
};

// ─────────────────────────────────────
static void pd4web_get(Pd4Web *x) {
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
static bool pd4web_check(Pd4Web *x) {
#if defined(_WIN32) || defined(_WIN64)
    // TODO: Need to implement

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
static void pd4web_setconfig(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
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
        x->gui = gui;
        if (x->gui) {
            post("[pd4web] Gui set to true");
        } else {
            post("[pd4web] Gui set to false");
        }
    } else if (config == "patch") {
        for (int i = 1; i < argc; i++) {
            if (argv[i].a_type != A_SYMBOL) {
                pd_error(x, "[pd4web] Invalid argument, use [set patch <patch_name>]");
                return;
            }
            x->patch += atom_getsymbolarg(i, argc, argv)->s_name;
            x->patch += " ";
        }

        // for ProjectRoot, get directory of the patch
        x->ProjectRoot = std::filesystem::path(x->patch).parent_path().string();
    }
    return;
}

// ─────────────────────────────────────
static void pd4web_browser(Pd4Web *x, float f) {
    if (!x->Pd4WebIsReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }

    if (x->patch == "") {
        pd_error(x, "[pd4web] No patch selected");
        return;
    }

    if (f == 1) {
        std::thread t([x]() {
            x->Server->set_mount_point("/", x->ProjectRoot.c_str());
            x->Server->Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_redirect("/index.html");
            });

            if (!x->Server->listen("0.0.0.0", 8080)) {
                post("[pd4web] Failed to start server");
            }
        });
        t.detach();
        post("[pd4web] Server started at http://localhost:8080");
        pdgui_vmess("::pd_menucommands::menu_openfile", "s", "http://localhost:8080");

    } else {
        x->Server->remove_mount_point("/");
        x->Server->stop();
    }
}
// ─────────────────────────────────────
static void pd4web_compile(Pd4Web *x) {
    if (!x->Pd4WebIsReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }

    std::string cmd = x->Pd4WebPath;

    if (x->verbose) {
        cmd += " --verbose ";
    }
    if (x->memory > 0) {
        cmd += " -m " + std::to_string(x->memory) + " ";
    }
    if (!x->gui) {
        cmd += " --nogui ";
    }
    if (x->patch != "") {
        cmd += x->patch;
    }

    std::thread t([cmd]() {
        post("[pd4web] Running...");
        std::array<char, 256> Buf;
        std::string Result;
        FILE *Pipe = popen(cmd.c_str(), "r");
        if (!Pipe) {
            pd_error(nullptr, "[pd4web] popen failed");
            return;
        }

        while (fgets(Buf.data(), Buf.size(), Pipe) != nullptr) {
            std::string Line = "[pd4web] ";
            Line += Buf.data();
            Line.erase(std::remove(Line.begin(), Line.end(), '\n'), Line.end());
            if (Line != "[pd4web] ") {
                post(Line.c_str());
            }
        }
        post("[pd4web] Done!");
        pclose(Pipe);
    });
    t.detach();

    return;
}

// ─────────────────────────────────────
static void *pd4web_new(t_symbol *s, int argc, t_atom *argv) {
    Pd4Web *x = (Pd4Web *)pd_new(pd4web_class);
    x->Out = outlet_new(&x->Obj, &s_anything);
    pd4web_get(x);
    x->Pd4WebPath = pd4web_class->c_externdir->s_name + x->Pd4WebExe;

    std::thread([x]() { x->Pd4WebIsReady = pd4web_check(x); }).detach();

    // default variables
    x->verbose = true;
    x->memory = 32;
    x->gui = true;

    // Server
    x->Server = new httplib::Server();

    return x;
}

// ─────────────────────────────────────
static void pd4web_free(Pd4Web *x) {
    x->Server->stop();
    delete x->Server;
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)pd4web_new, (t_method)pd4web_free,
                             sizeof(Pd4Web), CLASS_DEFAULT, A_GIMME, A_NULL);

    class_addmethod(pd4web_class, (t_method)pd4web_setconfig, gensym("set"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_browser, gensym("browser"), A_FLOAT, A_NULL);
    class_addbang(pd4web_class, (t_method)pd4web_compile);
}
