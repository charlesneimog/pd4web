#include <m_pd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <filesystem>
#include <string>
#include <thread>

#include <m_imp.h>

#include "./cpp-httplib/httplib.h"

static t_class *pd4web_class;

static std::atomic<bool> Pd4WebBrowser(false);
static std::atomic<bool> Pd4WebCompiler(false);

static std::atomic<bool> Pd4WebKillBrowser(false);

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
static std::thread::id RunCompiler(Pd4Web *x, std::string cmd) {
    std::thread t([cmd]() {
        post("[pd4web] Running...");
        Pd4WebBrowser.store(true);
        std::array<char, 256> Buf;
        std::string Result;
        FILE *Pipe = popen(cmd.c_str(), "r");
        if (!Pipe) {
            pd_error(nullptr, "[pd4web] popen failed");
            Pd4WebBrowser.store(false); // Ensure the flag is reset on failure
            return;
        }

        while (!Pd4WebKillBrowser.load() && fgets(Buf.data(), Buf.size(), Pipe) != nullptr) {
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
        Pd4WebBrowser.store(false);
    });

    std::thread::id threadId = t.get_id();
    t.detach();
    return threadId;
}

std::string getLocalIP() {
    char hostname[1024];
    struct hostent *host_entry;
    char *ip;

    // Get the hostname
    gethostname(hostname, sizeof(hostname));

    // Get the host information
    host_entry = gethostbyname(hostname);

    if (host_entry == nullptr) {
        std::cerr << "Error: Unable to get host information." << std::endl;
        return "";
    }

    // Convert the first IP address from binary to string
    ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));

    return std::string(ip);
}

// ─────────────────────────────────────
static void Browser(Pd4Web *x, float f) {
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
    } else {
        x->Server->stop();
    }
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
        for (int i = 1; i < argc; i++) {
            if (argv[i].a_type != A_SYMBOL) {
                pd_error(x, "[pd4web] Invalid argument, use [set patch <patch_name>]");
                return;
            }
            x->patch += atom_getsymbolarg(i, argc, argv)->s_name;
        }

        // for ProjectRoot, get directory of the patch
        x->ProjectRoot = std::filesystem::path(x->patch).parent_path().string();
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

    if (Pd4WebCompiler.load()) {
        pd_error(x, "[pd4web] Compiler is already running, wait");
        return;
    }

    std::thread t([cmd]() {
        post("[pd4web] Running...");
        Pd4WebCompiler.store(true);
        std::array<char, 256> Buf;
        std::string Result;
        FILE *Pipe = popen(cmd.c_str(), "r");
        if (!Pipe) {
            pd_error(nullptr, "[pd4web] popen failed");
            Pd4WebCompiler.store(false); // Ensure the flag is reset on failure
            return;
        }

        while (fgets(Buf.data(), Buf.size(), Pipe) != nullptr) {
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
        Pd4WebCompiler.store(false);
    });

    std::thread::id threadId = t.get_id();
    t.detach();

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
    x->Server = new httplib::Server();

    return x;
}

// ─────────────────────────────────────
static void Pd4WebFree(Pd4Web *x) {
    x->Server->stop();
    delete x->Server;
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)Pd4WebNew, (t_method)Pd4WebFree,
                             sizeof(Pd4Web), CLASS_DEFAULT, A_GIMME, A_NULL);

    class_addmethod(pd4web_class, (t_method)SetConfig, gensym("set"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)Browser, gensym("browser"), A_FLOAT, A_NULL);
    class_addbang(pd4web_class, (t_method)Compile);
}
