#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <thread>

#include <m_pd.h>

#include <m_imp.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <httplib.h>

static bool global_pd4web_check = false;
static t_class *pd4web_class;

#define PD4WEB_EXTERNAL_VERSION "2.2.5"

// ─────────────────────────────────────
class Pd4Web {
  public:
    t_object obj;

    // obj debug
    bool obj_debug;

    // constructor
    bool isReady;
    bool running;
    bool result;

    // commands
    std::string pip;
    std::string python;
    std::string pythonGlobal;
    std::string pd4web;

    // Terminal
    std::string cmd;

    // Server
    httplib::Server *server;
    std::string projectRoot;
    std::string objRoot;

    // compilation config
    bool cancel;
    std::string patch;
    std::string version;
    bool verbose;
    bool gui;
    bool debug;
    bool clear;
    int memory;
    int tpl;
    float zoom;
};

static bool pd4web_terminal(Pd4Web *x, std::string cmd, bool detached, bool sucessMsg,
                            bool showMessage, bool clearNewline);

#if _WIN32
std::string pd4web_pyexe(Pd4Web *x) {
    // Get the current user's home directory
    char *userProfile = getenv("USERPROFILE");
    if (!userProfile) {
        pd_error(x, "[pd4web] Unable to get USERPROFILE environment variable.");
        return "";
    }

    // Construct the base path
    std::filesystem::path basePath = std::string(userProfile);
    basePath /= "AppData\\Local\\Programs\\Python";

    // Check if the directory exists
    if (!std::filesystem::exists(basePath)) {
        pd_error(x, "[pd4web] Python directory not found at %s", basePath.string().c_str());
        return "";
    }

    for (const auto &entry : std::filesystem::directory_iterator(basePath)) {
        if (entry.is_directory()) {
            std::filesystem::path pythonPath = entry.path() / "python.exe";
            if (std::filesystem::exists(pythonPath)) {
                return pythonPath.string();
            }
        }
    }
    return "";
}
#endif

// ─────────────────────────────────────
#if defined(__APPLE__)
static std::string pd4web_terminal_info(Pd4Web *x, std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&std::fclose)> pipe(popen(cmd.c_str(), "r"), &std::fclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Read the output of the command
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // remove newline
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    // get real path, by default, result is a symlink
    std::filesystem::path path = std::filesystem::canonical(result);

    // make the path executable using chmod
    std::string chmod = "chmod +x " + path.string();
    int res = system(chmod.c_str());
    if (res != 0) {
        pd_error(x, "[pd4web] Failed to make the file executable");
    }

    return path.string();
}
#endif

// ─────────────────────────────────────
bool pd4web_terminal(Pd4Web *x, std::string cmd, bool detached = false, bool sucessMsg = false,
                     bool showMessage = false, bool clearNewline = false) {

    if (x->running) {
        pd_error(x, "[pd4web] Another command is running.");
        return false;
    }
    x->running = true;
    x->result = false;

#if defined(_WIN32) || defined(_WIN64)
    std::thread t([x, cmd, sucessMsg, showMessage, clearNewline]() {
        STARTUPINFOA si = {0};
        PROCESS_INFORMATION pi = {0};
        SECURITY_ATTRIBUTES sa = {0};
        HANDLE hRead, hWrite;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;

        // Create the pipe
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            pd_error(x, "[pd4web] Failed to create pipe!");
            x->running = false;
            x->result = false;
            return;
        }

        // Set up STARTUPINFO to redirect output to the pipe
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;

        // Create the process
        BOOL result = CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW,
                                     NULL, NULL, &si, &pi);
        if (result == 0) {
            DWORD error = GetLastError();
            x->running = false;
            x->result = false;
            return;
        }

        // Close write handle after creating the process
        CloseHandle(hWrite);

        char buffer[4096];
        DWORD bytesRead;
        std::string output;

        // Loop to read output from the pipe
        DWORD exitCode;
        while (true) {
            if (x->cancel) {
                TerminateProcess(pi.hProcess, 0);
                CloseHandle(hRead);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                x->running = false;
                x->result = false;
                x->cancel = false;
                pd_error(x, "[pd4web] Compilation canceled");
                break;
            }

            // Read data from the pipe
            BOOL readResult = ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL);
            if (readResult && bytesRead > 0) {
                output.append(buffer, bytesRead);
                if (showMessage && !output.empty()) {
                    output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());
                    if (output.find("ERROR:") != std::string::npos) {
                        pd_error(x, "%s", output.c_str());
                    } else {
                        post("%s", output.c_str());
                    }
                    output.clear();
                }
            }
            GetExitCodeProcess(pi.hProcess, &exitCode);
            if (exitCode != STILL_ACTIVE && bytesRead == 0) {
                break;
            }
        }
        if (sucessMsg && exitCode == 0) {
            post("[pd4web] Command executed successfully.");
        } else if (exitCode != 0) {
            x->running = false;
            x->result = false;
            if (showMessage) {
                pd_error(x, "[pd4web] Command failed with exit code %d", exitCode);
                post("Try to run the command '%s' in the terminal to see the error", cmd.c_str());
            }
        }
        CloseHandle(hRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        x->running = false;
        x->result = true;
    });
#else
    std::thread t([x, cmd, sucessMsg, showMessage, clearNewline]() {
        // post("[pd4web] Running command: %s", cmd.c_str());

        std::array<char, 8192> Buf;
        std::string Result;
        FILE *Pipe = popen(cmd.c_str(), "r");
        if (!Pipe) {
            pd_error(nullptr, "[pd4web] popen failed");
            x->running = false;
            x->result = false;
        }
        while (fgets(Buf.data(), Buf.size(), Pipe) != nullptr) {
            if (x->cancel) {
                x->running = false;
                x->result = false;
                x->cancel = false;
                pd_error(x, "[pd4web] Compilation canceled");
                break;
            }

            if (showMessage) {
                std::string line(Buf.data());
                if (clearNewline) {
                    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                    if (line != "") {
                        if (line.find("ERROR:") != std::string::npos) {
                            pd_error(x, "[pd4web] %s", line.c_str());
                        } else {
                            post("[pd4web] %s", line.c_str());
                        }
                    }
                } else {
                    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                    if (line != "") {
                        if (line.find("ERROR:") != std::string::npos) {
                            pd_error(x, "[pd4web] %s", line.c_str());
                        } else {
                            post("[pd4web] %s", line.c_str());
                        }
                    }
                }
            }
        }

        int exitCode = pclose(Pipe);
        if (exitCode != 0) {
            x->running = false;
            x->result = false;
            if (showMessage) {
                pd_error(x, "[pd4web] Command failed with exit code %d", exitCode);
                post("Try to run the command '%s' in the terminal to see the error", cmd.c_str());
            }
        } else {
            x->running = false;
            if (sucessMsg) {
                post("[pd4web] Command executed successfully.");
            }
            x->result = true;
        }
    });
#endif
    if (detached) {
        x->result = true;
        t.detach();
    } else {
        t.join();
    }
    return x->result;
}

// ─────────────────────────────────────
static void pd4web_debug_terminal(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    // loop through the arguments and create the command
    std::string cmd;
    for (int i = 0; i < argc; i++) {
        if (argv[i].a_type == A_SYMBOL) {
            cmd += atom_getsymbolarg(i, argc, argv)->s_name;
        } else if (argv[i].a_type == A_FLOAT) {
            cmd += std::to_string(atom_getfloatarg(i, argc, argv));
        } else {
            pd_error(x, "[pd4web] Invalid argument");
            return;
        }
        if (i < argc - 1) {
            cmd += " ";
        }
    }
    pd4web_terminal(x, cmd, false, false, true, false);
}

// ─────────────────────────────────────
static void pd4web_version(Pd4Web *x);

// ─────────────────────────────────────
static bool pd4web_check(Pd4Web *x) {
    if (!global_pd4web_check) {
        post("[pd4web] Checking pd4web installation...");
    }

    int result;
    std::string check_installation = x->python + " -c \"import pd4web\"";
    result = pd4web_terminal(x, check_installation, false, false, false, false);
    if (result) {
        pd4web_version(x);
        return true;
    } else {
        post("[pd4web] Installing pd4web, wait...");
    }
    // check if venv is installed
    std::string venv_installed = x->pythonGlobal + " -m venv --help";
    result = pd4web_terminal(x, venv_installed, false, false, false, false);
    if (!result) {
        std::string install_venv = x->pythonGlobal + " -m pip install virtualenv";
        post("Trying to install virtualenv...");
        result = pd4web_terminal(x, install_venv, false, false, false, false);
        if (!result) {
            pd_error(nullptr, "[pd4web] Failed to install virtualenv, please report this error on "
                              "https://github.com/charlesneimog/pd4web/issues");
            return false;
        }
    }

    // create virtual environment
    std::string objRoot = x->objRoot;
    std::string venv_cmd = x->pythonGlobal + " -m venv \"" + objRoot + "/.venv\"";
#ifdef _WIN32
    std::replace(objRoot.begin(), objRoot.end(), '\\', '/');
#endif
    post("[pd4web] Creating virtual environment...");
    result = pd4web_terminal(x, venv_cmd, false, false, false, false);
    if (!result) {
        pd_error(nullptr, "[pd4web] Failed to create virtual environment");
        return false;
    }
    // install pd4web
    post("[pd4web] Downloading pd4web...");
    std::string pip_cmd = x->pip + " install pd4web";
    result = pd4web_terminal(x, pip_cmd, false, false, false, false);
    if (!result) {
        pd_error(nullptr, "[pd4web] Failed to install pd4web");
        return false;
    }

    // check version
    std::string cmd = x->pd4web + " --version";
    pd4web_terminal(x, cmd.c_str(), false, false, true, true);
    global_pd4web_check = true;
    post("[pd4web] pd4web is ready, please restart Pd!");
    return true;
}

// ─────────────────────────────────────
static void pd4web_get(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    if (argv[0].a_type != A_SYMBOL) {
        pd_error(x, "[pd4web] Invalid argument, use [set <config_symbol> <value>]");
        return;
    }

    std::string config = atom_getsymbol(argv)->s_name;
    if (config == "version") {
        global_pd4web_check = false;
        pd4web_version(x);
        global_pd4web_check = true;
    }
}

// ─────────────────────────────────────
static void pd4web_set(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
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
        if (x->gui == gui) {
            return;
        }
        x->gui = gui;
        if (x->gui) {
            post("[pd4web] Gui set to true");
        } else {
            post("[pd4web] Gui set to false");
        }
    } else if ("zoom" == config) {
        float zoom = atom_getfloatarg(1, argc, argv);
        if (x->zoom == zoom) {
            return;
        }
        x->zoom = zoom;
        post("[pd4web] Zoom set to %.2f", x->zoom);
    } else if ("patch" == config) {
        std::string newpatch;
        for (int i = 1; i < argc; i++) {
            if (i > 1) {
                newpatch += " ";
            }
            if (argv[i].a_type == A_SYMBOL) {
                newpatch += atom_getsymbolarg(i, argc, argv)->s_name;
            } else if (argv[i].a_type == A_FLOAT) {
                newpatch += std::to_string(atom_getfloatarg(i, argc, argv));
            } else {
                pd_error(x, "[pd4web] Invalid argument, use [set patch <path>]");
            }
        }
        x->projectRoot = std::filesystem::path(newpatch).parent_path().string();
        x->patch = "\"" + newpatch + "\"";
    } else if ("template" == config) {
        int tpl = atom_getintarg(1, argc, argv);
        if (x->tpl == tpl) {
            return;
        }
        x->tpl = tpl;
        post("[pd4web] Template set to %d", x->tpl);
    } else if ("debug" == config) {
        bool debug = (bool)atom_getintarg(1, argc, argv);
        if (x->debug == debug) {
            return;
        }
        x->debug = debug;
        if (debug) {
            post("[pd4web] Debug set to true");
        } else {
            post("[pd4web] Debug set to false");
        }
    } else if ("clear" == config) {
        bool clear = (bool)atom_getintarg(1, argc, argv);
        if (x->clear == clear) {
            return;
        }
        if (clear) {
            x->clear = true;
            post("[pd4web] Clear set to true");
        } else {
            x->clear = false;
            post("[pd4web] Clear set to false");
        }
    } else if ("cancel" == config) {
        if (x->running) {
            x->cancel = true;
        } else {
            pd_error(x, "[pd4web] No compilation running");
        }
    } else {
        pd_error(x, "[pd4web] Invalid configuration");
    }

    return;
}

// ─────────────────────────────────────
static void pd4web_browser(Pd4Web *x, float f) {
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
            post("[pd4web] Root: %s", x->projectRoot.c_str());
            x->server->set_mount_point("/", x->projectRoot.c_str());
            x->server->Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_redirect("/index.html");
            });
            x->server->Get("/stop", [&](const httplib::Request &, httplib::Response &res) {
                post("[pd4web] Stopping server");
                x->server->stop();
            });
            std::string site = "http://localhost:8080";
            post("[pd4web] Starting server on %s", site.c_str());
            pdgui_vmess("::pd_menucommands::menu_openfile", "s", "http://localhost:8080");
            if (!x->server->listen("0.0.0.0", 8080)) {
                pd_error(x, "[pd4web] Failed to start server");
                return;
            }
        });
        t.detach();
    } else {
        httplib::Client client("http://localhost:8080");
        auto res = client.Get("/stop");
    }
}

// ─────────────────────────────────────
static void pd4web_version(Pd4Web *x) {
    std::string cmd = x->pd4web + " --version";
    if (global_pd4web_check) {
        pd4web_terminal(x, cmd.c_str(), true, false, false, true);
    } else {
        pd4web_terminal(x, cmd.c_str(), true, false, true, true);
        global_pd4web_check = true;
    }
}

// ─────────────────────────────────────
static void pd4web_clear_install(Pd4Web *x) {
    post("[pd4web] Uninstalling pd4web...");
    std::string cmd = x->pip + " uninstall pd4web -y";
    pd4web_terminal(x, cmd.c_str(), true);
    return;
}

// ─────────────────────────────────────
static void pd4web_update(Pd4Web *x, t_symbol *s, int argc, t_atom *argv) {
    std::string method = s->s_name;
    std::string mod;
    if (method == "git") {
        mod = "--pre pd4web --force-reinstall ";
    } else {
        mod = "pd4web --force-reinstall ";
    }
    std::string cmd = x->pip + " install " + mod + " --upgrade";
    pd_error(x, "[pd4web] Updating pd4web on background, please wait...");
    pd4web_terminal(x, cmd.c_str(), true, true, false, true);
    return;
}

// ─────────────────────────────────────
static void pd4web_compile(Pd4Web *x) {
    if (x->running) {
        pd_error(x, "[pd4web] Compilation already running");
        return;
    }
    if (!x->isReady) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }
    std::string cmd = x->pd4web + " --pd-external";
    cmd += " --pd-external-version \"" + x->version + "\" ";
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
    if (x->clear) {
        cmd += " --clear ";
    }
    if (x->debug) {
        cmd += " --debug ";
    }
    if (x->tpl != 0) {
        cmd += " --template " + std::to_string(x->tpl) + " ";
    }

    if (x->patch != "") {
        cmd += x->patch;
    } else {
        pd_error(x, "[pd4web] No patch selected");
        return;
    }

    if (x->running) {
        pd_error(x, "[pd4web] Compilation already running");
        return;
    }
    pd_error(x, "[pd4web] Compiling patch on background, please wait...");
    pd4web_terminal(x, cmd.c_str(), true, true, true, false);
    return;
}

// ─────────────────────────────────────
static void *pd4web_new(t_symbol *s, int argc, t_atom *argv) {
    Pd4Web *x = (Pd4Web *)pd_new(pd4web_class);
    x->objRoot = pd4web_class->c_externdir->s_name;
    x->running = false;

    for (int i = 0; i < argc; i++) {
        if (argv[i].a_type == A_SYMBOL) {
            std::string arg = atom_getsymbolarg(i, argc, argv)->s_name;
            if (arg == "-debug") {
                x->obj_debug = true;
            }
        }
    }

    x->pythonGlobal = "";
    std::string py_global = "python3 --version";
    bool result = pd4web_terminal(x, py_global, false, false, false, false);
    if (result) {
        x->pythonGlobal = "python3";
    } else {
        result = pd4web_terminal(x, "py --version", false, false, false, false);
        if (result) {
            x->pythonGlobal = "py";
        }
#ifdef _WIN32
        else {
            x->pythonGlobal = pd4web_pyexe(x);
        }
#endif
    }
    if (x->pythonGlobal == "") {
        pd_error(x, "[pd4web] Could not find Python 3. Please install Python 3, if installed "
                    "please report on https://github.com/charlesneimog/pd4web/issues");
        return nullptr;
    }

#ifdef _WIN32
    x->pip = "\"" + x->objRoot + "\\.venv\\Scripts\\pip.exe\"";
    x->python = "\"" + x->objRoot + "\\.venv\\Scripts\\python.exe\"";
    x->pd4web = "\"" + x->objRoot + "\\.venv\\Scripts\\pd4web.exe\"";
#elif defined(__APPLE__)
    std::string PATHS = "PATH=" + x->objRoot + "/.venv/bin:/usr/local/bin:/usr/bin:/bin";
    putenv((char *)PATHS.c_str());
    x->pip = "\"" + x->objRoot + "/.venv/bin/pip\"";
    x->python = "\"" + x->objRoot + "/.venv/bin/python\"";
    x->pd4web = "\"" + x->objRoot + "/.venv/bin/pd4web\"";
#elif defined(__linux__)
    x->pip = "\"" + x->objRoot + "/.venv/bin/pip\"";
    x->python = "\"" + x->objRoot + "/.venv/bin/python\"";
    x->pd4web = "\"" + x->objRoot + "/.venv/bin/pd4web\"";
#else
    pd_error(x, "[pd4web] Unsupported platform, please report this issue");
    return nullptr;
#endif
    std::thread([x]() { x->isReady = pd4web_check(x); }).detach();

    // pd4web config
    x->cancel = false;
    x->verbose = false;
    x->memory = 32;
    x->gui = true;
    x->zoom = 2;
    x->tpl = 0;
    x->version = PD4WEB_EXTERNAL_VERSION;
    x->server = new httplib::Server();

    if (x->obj_debug) {
        post("[pd4web] Python executable: %s", x->python.c_str());
    }

    if (!x->server->is_valid()) {
        pd_error(x, "[pd4web] Failed to create Server");
    }
    return x;
}

// ─────────────────────────────────────
static void pd4web_free(Pd4Web *x) {
    httplib::Client client("http://localhost:8080");
    auto res = client.Get("/stop");
    delete x->server;
    x->cancel = true;
    if (x->running) {
        post("[pd4web] Stopping pd4web...");
        while (x->running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        post("[pd4web] pd4web stopped");
    }
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)pd4web_new, (t_method)pd4web_free,
                             sizeof(Pd4Web), CLASS_DEFAULT, A_GIMME, A_NULL);

    class_addmethod(pd4web_class, (t_method)pd4web_get, gensym("get"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("set"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_browser, gensym("browser"), A_FLOAT, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("update"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("git"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_clear_install, gensym("uninstall"), A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_debug_terminal, gensym("_run"), A_GIMME, A_NULL);
    class_addbang(pd4web_class, (t_method)pd4web_compile);
}
