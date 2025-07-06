#include <algorithm>
#include <array>
#include <filesystem>
#include <string>

#ifndef _POSIX_SEM_VALUE_MAX
#define _POSIX_SEM_VALUE_MAX 32767
#endif
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <cstdlib> // for getenv
#include <windows.h>
#endif

#include <m_pd.h>

#include <m_imp.h>

#include <httplib.h>

static t_class *pd4web_class;

static bool global_pd4web_check = false; // just need to check once
#define PD4WEB_EXTERNAL_VERSION "2.4.0"

// ─────────────────────────────────────
class Pd4Web {
  public:
    t_object obj;

    // constructor
    bool is_ready;
    bool running;
    bool result;

    // executables
    std::string pip;
    std::string python_env;
    std::string python_global;
    std::string pd4web;

    // server
    httplib::Server *server;
    std::string project_root;
    std::string object_root;

    // compilation config
    std::string cmd;
    std::string patch;
    std::string version;

    bool verbose;
    bool gui;
    bool debug;
    bool clear;
    bool cancel;

    int memory;
    int patch_template;
    float zoom;
};

// ─────────────────────────────────────
static bool pd4web_terminal(Pd4Web *x, std::string cmd, bool detached = false,
                            bool sucessMsg = false, bool showMessage = false,
                            bool clearNewline = false) {

    logpost(x, 3, "[pd4web] Running command: %s", cmd.c_str());
    if (x->running) {
        pd_error(x, "[pd4web] Another command is running, please wait.");
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
                        logpost(x, 2, "%s", output.c_str());
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
            logpost(x, 2, "[pd4web] Command executed successfully.");
        } else if (exitCode != 0) {
            x->running = false;
            x->result = false;
            if (showMessage) {
                pd_error(x, "[pd4web] Command failed with exit code %d", exitCode);
                logpost(x, 3, "Try to run the command '%s' in the terminal to see the error",
                        cmd.c_str());
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
        std::array<char, 8192> Buf;
        std::string Result;
        FILE *Pipe = popen(cmd.c_str(), "r");
        if (!Pipe) {
            pd_error(x, "[pd4web] popen failed");
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
                            logpost(x, 2, "[pd4web] %s", line.c_str());
                        }
                    }
                } else {
                    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                    if (line != "") {
                        if (line.find("ERROR:") != std::string::npos) {
                            pd_error(x, "[pd4web] %s", line.c_str());
                        } else {
                            logpost(x, 2, "[pd4web] %s", line.c_str());
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
                logpost(x, 3, "Try to run the command '%s' in the terminal to see the error",
                        cmd.c_str());
            }
        } else {
            x->running = false;
            if (sucessMsg) {
                logpost(x, 2, "[pd4web] Command executed successfully.");
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
static bool pd4web_pyexe(Pd4Web *x) {
#if defined(__APPLE__) || defined(__linux__)
    if (pd4web_terminal(x, "python3 --version", false, false, false, false)) {
        x->python_global = "python3";
        logpost(x, 3, "[pd4web] python3 found");
        return true;
    } else if (pd4web_terminal(x, "python --version", false, false, false, false)) {
        x->python_global = "python";
        logpost(x, 3, "[pd4web] py found");
        return true;
    } else {
        pd_error(x, "[pd4web] Python 3 not found, please install Python 3");
        return false;
    }
#else
    if (pd4web_terminal(x, "py --version", false, false, false, false)) {
        x->python_global = "py";
        logpost(x, 3, "[pd4web] py found");
        return true;
    } else if (pd4web_terminal(x, "python --version", false, false, false, false)) {
        x->python_global = "python";
        return true;
    }

    post("[pd4web] Searching for Python Launcher...");
    char *userProfile = getenv("USERPROFILE");
    if (!userProfile) {
        pd_error(x, "[pd4web] Unable to get USERPROFILE environment variable.");
        return false;
    }
    std::string py_exe =
        std::string(userProfile) + "\\AppData\\Local\\Programs\\Python\\Launcher\\py.exe";
    if (std::filesystem::exists(py_exe)) {
        x->python_global = py_exe;
        return true;
    } else {
        pd_error(x, "[pd4web] Python Launcher not found, please install Python 3, if you have "
                    "Python 3 installed, "
                    "please report using https://github.com/charlesneimog/pd4web/issues/new");
        return false;
    }
#endif
}

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
static bool pd4web_check(Pd4Web *x) {
    if (!global_pd4web_check) {
        logpost(x, 2, "[pd4web] Checking pd4web installation...");
    }

    int result;
    std::string check_installation = x->python_env + " -c \"import pd4web\"";
    result = pd4web_terminal(x, check_installation, false, false, false, false);
    if (result) {
        std::string cmd = x->pd4web + " --version";
        pd4web_terminal(x, cmd.c_str(), false, false, true, true);
        return true;
    } else {
        logpost(x, 2, "[pd4web] Installing pd4web, wait...");
    }
    // check if venv is installed
    std::string venv_installed = x->python_global + " -m venv --help";
    result = pd4web_terminal(x, venv_installed, false, false, false, false);
    if (!result) {
        std::string install_venv = x->python_global + " -m pip install virtualenv";
        logpost(x, 2, "[pd4web] Trying to install virtualenv...");
        result = pd4web_terminal(x, install_venv, false, false, false, false);
        if (!result) {
            pd_error(x, "[pd4web] Failed to install virtualenv, please report this error on "
                        "https://github.com/charlesneimog/pd4web/issues");
            return false;
        }
    }

    // create virtual environment
    std::string object_root = x->object_root;
    std::string venv_cmd = x->python_global + " -m venv \"" + object_root + "/.venv\"";
#ifdef _WIN32
    std::replace(object_root.begin(), object_root.end(), '\\', '/');
#endif
    logpost(x, 2, "[pd4web] Creating virtual environment...");
    result = pd4web_terminal(x, venv_cmd, false, false, false, false);
    if (!result) {
        pd_error(x, "[pd4web] Failed to create virtual environment");
        return false;
    }
    // install pd4web
    logpost(x, 2, "[pd4web] Downloading pd4web...");
    std::string pip_cmd = x->pip + " install pd4web";
    result = pd4web_terminal(x, pip_cmd, false, false, false, false);
    if (!result) {
        pd_error(x, "[pd4web] Failed to install pd4web");
        return false;
    }

    // check version
    std::string cmd = x->pd4web + " --version";
    pd4web_terminal(x, cmd.c_str(), false, false, true, true);
    global_pd4web_check = true;
    logpost(x, 2, "[pd4web] pd4web is ready, please restart Pd!");
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
            logpost(x, 2, "[pd4web] Initial memory set to %dMB", x->memory);
        }
        return;
    } else if ("verbose" == config) {
        bool verbose = atom_getintarg(1, argc, argv);
        if (verbose != x->verbose) {
            x->verbose = verbose;
            if (x->verbose) {
                logpost(x, 2, "[pd4web] Verbose set to true");
            } else {
                logpost(x, 2, "[pd4web] Verbose set to false");
            }
        }
    } else if ("gui" == config) {
        bool gui = atom_getintarg(1, argc, argv);
        if (x->gui == gui) {
            return;
        }
        x->gui = gui;
        if (x->gui) {
            logpost(x, 2, "[pd4web] Gui set to true");
        } else {
            logpost(x, 2, "[pd4web] Gui set to false");
        }
    } else if ("zoom" == config) {
        float zoom = atom_getfloatarg(1, argc, argv);
        if (x->zoom == zoom) {
            return;
        }
        x->zoom = zoom;
        logpost(x, 2, "[pd4web] Zoom set to %.2f", x->zoom);
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
        x->project_root = std::filesystem::path(newpatch).parent_path().string();
        x->patch = "\"" + newpatch + "\"";
    } else if ("template" == config) {
        int patch_template = atom_getintarg(1, argc, argv);
        if (x->patch_template == patch_template) {
            return;
        }
        x->patch_template = patch_template;
        logpost(x, 2, "[pd4web] Template set to %d", x->patch_template);
    } else if ("debug" == config) {
        bool debug = (bool)atom_getintarg(1, argc, argv);
        if (x->debug == debug) {
            return;
        }
        x->debug = debug;
        if (debug) {
            logpost(x, 2, "[pd4web] Debug set to true");
        } else {
            logpost(x, 2, "[pd4web] Debug set to false");
        }
    } else if ("clear" == config) {
        bool clear = (bool)atom_getintarg(1, argc, argv);
        if (x->clear == clear) {
            return;
        }
        if (clear) {
            x->clear = true;
            logpost(x, 2, "[pd4web] Clear set to true");
        } else {
            x->clear = false;
            logpost(x, 2, "[pd4web] Clear set to false");
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
    if (!x->is_ready) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }

    if (x->patch == "") {
        pd_error(x, "[pd4web] No patch selected");
        return;
    }

    if (f == 1) {
        std::thread t([x]() {
            logpost(x, 3, "[pd4web] Root: %s", x->project_root.c_str());
            x->server->set_mount_point("/", x->project_root.c_str());
            x->server->Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_redirect("/index.html");
            });
            x->server->Get("/stop", [&](const httplib::Request &, httplib::Response &res) {
                logpost(x, 2, "[pd4web] Stopping server");
                x->server->stop();
            });
            std::string site = "http://localhost:8080";
            logpost(x, 2, "[pd4web] Starting server on %s", site.c_str());
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
static void pd4web_clear_install(Pd4Web *x) {
    logpost(x, 2, "[pd4web] Uninstalling pd4web...");
    std::string cmd = x->pip + " uninstall pd4web -y";
    pd4web_terminal(x, cmd.c_str(), true, true, true, true);
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
    logpost(x, 2, "[pd4web] Updating pd4web on background, please wait...");
    pd4web_terminal(x, cmd.c_str(), true, true, false, true);
    return;
}

// ─────────────────────────────────────
static void pd4web_compile(Pd4Web *x) {
    if (x->running) {
        pd_error(x, "[pd4web] Compilation already running");
        return;
    }
    if (!x->is_ready) {
        pd_error(x, "[pd4web] pd4web is not ready");
        return;
    }
    std::string cmd = x->pd4web + " --pd-external";
    std::string version = PD4WEB_EXTERNAL_VERSION;
    cmd += " --pd-external-version \"" + version + "\" ";
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
    if (x->patch_template != 0) {
        cmd += " --template " + std::to_string(x->patch_template) + " ";
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
    logpost(x, 2, "[pd4web] Compiling patch on background, please wait...");
    pd4web_terminal(x, cmd.c_str(), true, true, true, false);
    return;
}

// ─────────────────────────────────────
static void *pd4web_new() {
    Pd4Web *x = (Pd4Web *)pd_new(pd4web_class);
    if (!x) {
        pd_error(x, "[pd4web] Could not create object, memory allocation failed");
        return nullptr;
    }
    x->object_root = pd4web_class->c_externdir->s_name;
    x->running = false;
    x->python_global = "not found";

#ifdef _WIN32
    x->pip = "\"" + x->object_root + "\\.venv\\Scripts\\pip.exe\"";
    x->python_env = "\"" + x->object_root + "\\.venv\\Scripts\\python.exe\"";
    x->pd4web = "\"" + x->object_root + "\\.venv\\Scripts\\pd4web.exe\"";
    std::replace(x->pip.begin(), x->pip.end(), '/', '\\');
    std::replace(x->python_env.begin(), x->python_env.end(), '/', '\\');
    std::replace(x->pd4web.begin(), x->pd4web.end(), '/', '\\');

#elif defined(__APPLE__)
    std::string PATHS = "PATH=" + x->object_root + "/.venv/bin:/usr/local/bin:/usr/bin:/bin";
    putenv((char *)PATHS.c_str());
    x->pip = "\"" + x->object_root + "/.venv/bin/pip\"";
    x->python_env = "\"" + x->object_root + "/.venv/bin/python\"";
    x->pd4web = "\"" + x->object_root + "/.venv/bin/pd4web\"";

#elif defined(__linux__)
    x->pip = "\"" + x->object_root + "/.venv/bin/pip\"";
    x->python_env = "\"" + x->object_root + "/.venv/bin/python\"";
    x->pd4web = "\"" + x->object_root + "/.venv/bin/pd4web\"";

#else
    pd_error(x, "[pd4web] Unsupported platform, please report this issue");
    return nullptr;

#endif
    bool ok = pd4web_pyexe(x);
    if (!ok && x->python_env == "not found") {
        pd_error(x, "[pd4web] Could not find Python 3. Please install Python 3, if installed "
                    "please report on https://github.com/charlesneimog/pd4web/issues");
        return nullptr;
    }

    std::thread([x]() { x->is_ready = pd4web_check(x); }).detach();

    // pd4web config
    x->cancel = false;
    x->verbose = false;
    x->memory = 32;
    x->gui = true;
    x->zoom = 2;
    x->patch_template = 0;
    x->version = PD4WEB_EXTERNAL_VERSION;
    x->server = new httplib::Server();

    logpost(x, 3, "[pd4web] python: %s", x->python_global.c_str());
    logpost(x, 3, "[pd4web] python venv executable: %s", x->python_env.c_str());
    logpost(x, 3, "[pd4web] pip executable: %s", x->pip.c_str());
    logpost(x, 3, "[pd4web] pd4web executable: %s", x->pd4web.c_str());

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
        logpost(x, 2, "[pd4web] Stopping pd4web...");
        while (x->running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        logpost(x, 2, "[pd4web] pd4web stopped");
    }
}

// ─────────────────────────────────────
extern "C" void pd4web_setup(void) {
    pd4web_class = class_new(gensym("pd4web"), (t_newmethod)pd4web_new, (t_method)pd4web_free,
                             sizeof(Pd4Web), CLASS_DEFAULT, A_NULL);

    class_addmethod(pd4web_class, (t_method)pd4web_get, gensym("get"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("set"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_browser, gensym("browser"), A_FLOAT, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("update"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("git"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_update, gensym("dev"), A_GIMME, A_NULL);
    class_addmethod(pd4web_class, (t_method)pd4web_clear_install, gensym("uninstall"), A_NULL);
    // class_addmethod(pd4web_class, (t_method)pd4web_debug_terminal, gensym("_run"), A_GIMME,
    // A_NULL);

    class_addbang(pd4web_class, (t_method)pd4web_compile);
}
