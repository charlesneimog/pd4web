#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <thread>

#include <m_pd.h>
#include <m_imp.h>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#endif

#include "./cpp-httplib/httplib.h"

static t_class *pd4web_class;

// ─────────────────────────────────────
class Pd4Web {
  public:
    t_object obj;
    
    bool isReady;
    bool running;

    // Terminal
    std::string cmd;

    // Server
    httplib::Server *server;
    std::string projectRoot;
    std::string objRoot;

    // config
    std::string patch;
    bool verbose;
    bool gui;
    bool debug;
    bool clear;

    int memory;
    int tpl;

    float zoom;

    t_outlet *Out;
};

//
static void pd4web_version(Pd4Web *x);


// ─────────────────────────────────────
static int pd4web_terminal(Pd4Web *x, std::string cmd, bool detached=false, bool sucessMsg=false) {
    x->running = true;
    
#if defined(_WIN32) 
    std::thread t([x, cmd, sucessMsg]() {
        std::array<char, 256> Buf;
        std::string Result;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        SECURITY_ATTRIBUTES sa;
        HANDLE hRead, hWrite;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESTDHANDLES;

        ZeroMemory(&pi, sizeof(pi));

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            pd_error(nullptr, "[pd4web] CreatePipe failed");
            x->running = false;
            return;
        }

        si.hStdOutput = hWrite;
        si.hStdError = hWrite;

        if (!CreateProcess(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            pd_error(nullptr, "[pd4web] CreateProcess failed");
            x->running = false;
            CloseHandle(hWrite);
            CloseHandle(hRead);
            return;
        }

        CloseHandle(hWrite);

        std::string LastLine = "";
        DWORD bytesRead;
        while (ReadFile(hRead, Buf.data(), Buf.size(), &bytesRead, NULL) && bytesRead > 0) {
            std::string Line = "[pd4web] ";
            Line.append(Buf.data(), bytesRead);
            Line.erase(std::remove(Line.begin(), Line.end(), '\n'), Line.end());
            if (Line != "[pd4web] ") {
                LastLine = Line;
            }
        }

        post(LastLine.c_str());

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(hRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (exitCode != 0) {
            x->running = false;
            pd_error(nullptr,
                    "[pd4web] Command failed, please run '%s' in the terminal to check more details",
                        cmd.c_str());
        } else {
            x->running = false;
            if (sucessMsg) {
                pd_error(x, "[pd4web] Done!");
            }
        }
    });
    if (detached) {
        t.detach();
    } else {
        t.join();
    }

#else
    std::thread t([x, cmd, sucessMsg]() {
        std::array<char, 256> Buf;
        std::string Result;
        FILE *Pipe = popen(cmd.c_str(), "r");
        if (!Pipe) {
            pd_error(nullptr, "[pd4web] popen failed");
            x->running = false;
            return;
        }
        std::string LastLine = "";
        while (fgets(Buf.data(), Buf.size(), Pipe) != nullptr) {
            std::string Line = "[pd4web] ";
            Line += Buf.data();
            Line.erase(std::remove(Line.begin(), Line.end(), '\n'), Line.end());
            if (Line != "[pd4web] ") {
                LastLine = Line;
            }
        }
        post(LastLine.c_str());
        int exitCode = pclose(Pipe);
        if (exitCode != 0) {
            x->running = false;
            pd_error(nullptr,
                     "[pd4web] Command failed, please run '%s' in the terminal to check more details",
                        cmd.c_str());
        } else {
            x->running = false;
            if (sucessMsg) {
                pd_error(x, "[pd4web] Done!");
            }
        }
    });

    if (detached) {
        t.detach();
    } else {
        t.join();
    }
#endif
    return 0;
}

// ─────────────────────────────────────
static bool pd4web_check(Pd4Web *x) {
    int result;
#if defined(_WIN32) || defined(_WIN64)
    std::string check_installation =
        "\"" + x->objRoot + "/.venv/Scripts/python.exe\" -c \"import pd4web\"";
    result = pd4web_terminal(x, check_installation.c_str());
    if (result == 0) {
        pd4web_version(x);
        //post("[pd4web] pd4web is ready!");
        return true;
    }

    result = pd4web_terminal(x, "python --version > NUL 2>&1");
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Python is not installed. Please install Python first.");
        return false;
    }

    post("[pd4web] Creating virtual environment...");
    std::string venv_cmd = "python -m venv \"" + x->objRoot + "\\.venv\"";
    result = pd4web_terminal(x, venv_cmd.c_str());
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Failed to create virtual environment");
        return false;
    }

    // install pd4web
    post("[pd4web] Installing pd4web...");
    std::string pip_cmd = "\"" + x->objRoot + "\\.venv\\Scripts\\pip.exe\" install pd4web";
    result = pd4web_terminal(x, pip_cmd.c_str());
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Failed to install pd4web");
        return false;
    }
    pd4web_version(x);
    //post("[pd4web] pd4web is ready!");
    return true;
#else
    // check if python3 is installed
    result = std::system("python3 --version > /dev/null 2>&1");
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Python 3 is not installed. Please install Python first.");
        return false;
    }
    std::string venv_cmd = "python3 -m venv \"" + x->objRoot + "/.venv\"";
    result = std::system(venv_cmd.c_str());
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Failed to create virtual environment");
        return false;
    }
    // install pd4web
    std::string pip_cmd = "\"" + x->objRoot + "/.venv/bin/pip\" install pd4web";
    result = std::system(pip_cmd.c_str());
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Failed to install pd4web");
        return false;
    }
    pd4web_version(x);

    //post("[pd4web] pd4web is ready!");
    return true;
#endif
}

// ─────────────────────────────────────
static void pd4web_setconfig(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    if (x->running) {
        pd_error(
            x, "[pd4web] pd4web is running, please wait it to finish before change configurations");
        return;
    }

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
    } else if ("verbose" == config) {
        bool verbose = atom_getintarg(1, argc, argv);
        if (verbose != x->verbose) {
            x->verbose = verbose;
            if (x->verbose) {
                post("[pd4web] Verbose set to true");
            } else {
                post("[pd4web] Verbose set to false");
            }
        }
    } else if ("gui" == config) {
        bool gui = atom_getintarg(1, argc, argv);
        if (x->gui == gui){ return ;}
        x->gui = gui;
        if (x->gui) {
            post("[pd4web] Gui set to true");
        } else {
            post("[pd4web] Gui set to false");
        }
    } else if ("zoom" == config) {
        float zoom = atom_getfloatarg(1, argc, argv);
        if (x->zoom == zoom){ return ;}
        x->zoom = zoom;
        post("[pd4web] Zoom set to %.2f", x->zoom);
    } else if ("patch" == config) {
        std::string newpatch;
        for (int i = 1; i < argc; i++) {
            if (argv[i].a_type != A_SYMBOL) {
                pd_error(x, "[pd4web] Invalid argument, use [set patch <patch_name>]");
                return;
            }
            newpatch += atom_getsymbolarg(i, argc, argv)->s_name;
            newpatch += " ";
        }
        x->projectRoot = std::filesystem::path(newpatch).parent_path().string();
        x->patch = newpatch;
    } else if ("template" == config) {
        int tpl = atom_getintarg(1, argc, argv);
        if (x->tpl == tpl){return;}
        x->tpl = tpl;
        post("[pd4web] Template set to %d", x->tpl);
    } else if ("debug" == config) {
        bool debug = (bool)atom_getintarg(1, argc, argv);
        if (x->debug == debug){ return ;}
        x->debug = debug;
        if (debug){
            post("[pd4web] Debug set to true");
        } else {
            post("[pd4web] Debug set to false");
        }
    } else if ("clear" == config) {
        int clear = atom_getintarg(1, argc, argv);
        if (x->clear == clear){ return ;}
        if (clear){
            x->clear = true;
            post("[pd4web] Clear set to true");
        } else {
            x->clear = false;
            post("[pd4web] Clear set to false");
        }
    }
    else{
        pd_error(x, "[pd4web] Invalid configuration");
    }

    return;
}

// ─────────────────────────────────────
static void pd4web_browser(Pd4Web *x, float f) {
    if (x->running && f == 1) {
        pd_error(x, "[pd4web] pd4web is running, please wait it to finish before open the browser");
        return;
    }
    if (!x->isReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }

    if (x->patch == "") {
        pd_error(x, "[pd4web] No patch selected");
        return;
    }

    if (f == 1) {
        std::thread t([x]() {
            x->server->set_mount_point("/", x->projectRoot.c_str());
            x->server->Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_redirect("/index.html");
            });
            if (!x->server->listen("0.0.0.0", 8080)) {
                post("[pd4web] Failed to start server");
            }
        });
        t.detach();
        post("[pd4web] Server started at http://localhost:8080");
        pdgui_vmess("::pd_menucommands::menu_openfile", "s", "http://localhost:8080");
    } else {
        x->server->remove_mount_point("/");
        x->server->stop();
    }
}

// ─────────────────────────────────────
static void pd4web_version(Pd4Web *x){
    #if defined(_WIN32) || defined(_WIN64)
        std::string cmd = "\"" + x->objRoot + "/.venv/Scripts/pd4web.exe\" --version";
        pd_error(x, "Not implemented on Windows yet, please run '%s' in the terminal", cmd.c_str());
        return;
    #else
        std::string cmd = x->objRoot + "/.venv/bin/pd4web --version";
        pd4web_terminal(x, cmd.c_str(), true);
    #endif
    
}

// ─────────────────────────────────────
static void pd4web_clear_install(Pd4Web *x){
    post("[pd4web] Uninstalling pd4web...");
#if defined(_WIN32) || defined(_WIN64)
    std::string cmd = "\"" + x->objRoot + "/.venv/Scripts/pip.exe\" uninstall pd4web -y";
#else
    std::string cmd = x->objRoot + "/.venv/bin/pip uninstall pd4web -y";
#endif
    pd4web_terminal(x, cmd.c_str(), true);
    return;
}

// ─────────────────────────────────────
static void pd4web_update(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    std::string method = s->s_name;
    std::string mod;
   if (method == "git") {
        //post("[pd4web] Updating pd4web from git...");
        mod = "--pre pd4web --force-reinstall ";
    } else {
        mod = "pd4web --force-reinstall ";
        //post("[pd4web] Updating pd4web from pip...");
    }

    // check if git is installed
    int result = std::system("git --version > /dev/null 2>&1");
    if (result != 0) {
        pd_error(nullptr, "[pd4web] Git is not installed. Please install Git first to use the git version.");
        return;
    }

#if defined(_WIN32) || defined(_WIN64)
    std::string cmd = "\"" + x->objRoot + "/.venv/Scripts/pip.exe\" install " + mod  + " --upgrade";
#else
    std::string cmd = x->objRoot + "/.venv/bin/pip install " + mod + " --upgrade";
#endif
#if defined(_WIN32) || defined(_WIN64)
    std::thread t([x, cmd]() {
        int result = system(cmd.c_str());
        if (result != 0) {
            pd_error(nullptr,
                     "[pd4web] Update failed, please report this issue to the pd4web repository");
            x->running = false;
            return;
        } else {
            x->running = false;
            post("[pd4web] Done!");
        }
    });
    t.detach();
#else
    pd_error(x, "[pd4web] Updating pd4web on background, please wait...");
    pd4web_terminal(x, cmd.c_str(), true);
#endif
    return;
}

// ─────────────────────────────────────
static void pd4web_compile(Pd4Web *x) {
    x->running = true;
    if (!x->isReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        x->running = false;
        return;
    }

    // pd4web bin
#if defined(_WIN32) || defined(_WIN64)
    std::string cmd = "\"" + x->objRoot + "/.venv/Scripts/pd4web.exe\" ";
#else
    std::string cmd = "\"" + x->objRoot + "/.venv/bin/pd4web\" ";
    cmd += "--pd-external ";
#endif

    if (x->verbose) {
        cmd += " --verbose ";
    }
    if (x->memory > 0) {
        cmd += " -m " + std::to_string(x->memory) + " ";
    }
    if (!x->gui) {
        cmd += " --nogui ";
    }
    if (x->zoom != 1) {
        cmd += " --patch-zoom " + std::to_string(x->zoom) + " ";
    }
    if (x->clear){
        cmd += " --clear ";
    }
    if (x->debug){
        cmd += " --debug ";
    }
    if (x->tpl != 0){
        cmd += " --template " + std::to_string(x->tpl) + " ";
    }

    if (x->patch != "") {
        cmd += x->patch;
    }
    
#if defined(_WIN32) || defined(_WIN64)
    std::thread t([x, cmd]() {
        // here we show the terminal window
        int result = system(cmd.c_str());
        if (result != 0) {
            pd_error(nullptr,
                     "[pd4web] Command failed, if this is the first time you are running pd4web, "
                     "please run '%s' in the terminal",
                     cmd.c_str());
            x->running = false;
            return;
        } else {
            x->running = false;
            post("[pd4web] Done!");
        }
    });
    t.detach();
#else
    pd_error(x, "[pd4web] Compiling patch on background, please wait...");
    pd4web_terminal(x, cmd.c_str(), true, true);
#endif

    return;
}

// ─────────────────────────────────────
static void *pd4web_new(t_symbol *s, int argc, t_atom *argv) {
    Pd4Web *x = (Pd4Web *)pd_new(pd4web_class);
    x->Out = outlet_new(&x->obj, &s_anything);
    x->objRoot = pd4web_class->c_externdir->s_name;
    x->running = false;
    post("[pd4web] Checking pd4web...");
    std::thread([x]() { x->isReady = pd4web_check(x); }).detach();

    // default variables
    x->verbose = false;
    x->memory = 32;
    x->gui = true;
    x->zoom = 2;

    x->server = new httplib::Server();
    return x;
}

// ─────────────────────────────────────
static void pd4web_free(Pd4Web *x) {
    x->server->stop();
    delete x->server;
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)pd4web_new, (t_method)pd4web_free,
                             sizeof(Pd4Web), CLASS_DEFAULT, A_GIMME, A_NULL);

    class_addmethod(pd4web_class, (t_method)pd4web_setconfig, gensym("set"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_browser, gensym("browser"), A_FLOAT, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("update"), A_GIMME,0);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("git"), A_GIMME,0);
    class_addmethod(pd4web_class, (t_method)pd4web_version, gensym("version"), A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_clear_install, gensym("uninstall"), A_NULL);
    class_addbang(pd4web_class, (t_method)pd4web_compile);
}
