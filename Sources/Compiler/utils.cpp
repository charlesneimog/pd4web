#include "pd4web_compiler.hpp"

#define BOOST_PROCESS_VERSION 2
#include <array>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#endif

namespace bp = boost::process::v2;
namespace asio = boost::asio;

// ─────────────────────────────────────
std::string Pd4Web::getCertFile() {
#if defined(_WIN32)
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        int python_minor = 10;
        int python_major = 99;
        std::filesystem::path certPath;
        for (int i = python_minor; i < python_major; i++) {
            std::filesystem::path possiblePath = std::filesystem::path(path) / L"Programs" /
                                                 L"Python" / (L"Python3" + std::to_wstring(i)) /
                                                 L"Lib" / L"site-packages" / L"certifi" /
                                                 L"cacert.pem";

            if (fs::exists(possiblePath)) {
                certPath = possiblePath;
                break;
            }
        }
        if (certPath.empty()) {
            print("Certificate not found, installing certifi package for SSL certificates...",
                  Pd4WebLogLevel::PD4WEB_LOG2);
            std::vector<std::string> pipInstallCmd = {"-m", "pip", "install", "certifi"};
            int pipResult = execProcess(m_PythonWindows, pipInstallCmd);
            if (pipResult != 0) {
                print("Failed to install certifi package via pip. SSL connections may fail.",
                      Pd4WebLogLevel::PD4WEB_WARNING);
            } else {
                print("certifi package installed successfully.", Pd4WebLogLevel::PD4WEB_LOG2);
                print("Please restart the Pd/Python", Pd4WebLogLevel::PD4WEB_ERROR);
            }
            return {};
        }

        // Convert wide to UTF-8
        int size_needed =
            WideCharToMultiByte(CP_UTF8, 0, certPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0) {
            print("Failed to convert certificate path to UTF-8", Pd4WebLogLevel::PD4WEB_WARNING);
            return {};
        }
        std::string certPathUtf8(size_needed - 1, 0);
        if (WideCharToMultiByte(CP_UTF8, 0, certPath.c_str(), -1, certPathUtf8.data(), size_needed,
                                nullptr, nullptr) == 0) {
            print("Failed to convert certificate path to UTF-8", Pd4WebLogLevel::PD4WEB_WARNING);
            return {};
        }
        return certPathUtf8;
    }
    return {};

#else
    std::string result = "";
    static const std::vector<std::string> cafiles = {
        "/etc/ssl/certs/ca-certificates.crt", "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/ssl/cert.pem", "/etc/ssl/certs/ca-bundle.crt"};

    for (const auto &candidate : cafiles) {
        std::error_code ecCandidate;
        if (fs::exists(candidate, ecCandidate)) {
            print("Certificate file found: " + candidate, Pd4WebLogLevel::PD4WEB_VERBOSE);
            return candidate;
        }
    }
    print("Certificate file not found", Pd4WebLogLevel::PD4WEB_WARNING);

    return {};
#endif
}

// ─────────────────────────────────────
int Pd4Web::execProcess(const std::string &command, std::vector<std::string> &args) {
    std::ostringstream oss;
    for (const auto &a : args) {
        oss << a << ' ';
    }
    print(command + " " + oss.str());

#if defined(__linux__) || defined(__APPLE__)
    std::string certPath = getCertFile();
    asio::io_context ctx;
    boost::asio::readable_pipe out{ctx}, err{ctx};
    bp::process proc(ctx, command, args, bp::process_stdio{.in = {}, .out = out, .err = err});

    auto read_loop = [&](boost::asio::readable_pipe &pipe) {
        std::array<char, 4096> buf;
        for (;;) {
            boost::system::error_code ec;
            std::size_t n = pipe.read_some(asio::buffer(buf), ec);
            if (ec == boost::asio::error::eof) {
                break;
            }
            if (ec) {
                throw boost::system::system_error(ec);
            }
            if (n == 0) {
                continue;
            }
            std::string_view sv(buf.data(), n);
            if (!sv.empty() && sv.back() == '\n') {
                sv.remove_suffix(1);
            }
            if (!sv.empty()) {
                print(std::string(sv), Pd4WebLogLevel::PD4WEB_LOG2);
            }
        }
    };

    std::exception_ptr ex_out, ex_err;
    std::thread t_out([&] {
        try {
            read_loop(out);
        } catch (...) {
            ex_out = std::current_exception();
        }
    });
    std::thread t_err([&] {
        try {
            read_loop(err);
        } catch (...) {
            ex_err = std::current_exception();
        }
    });

    t_out.join();
    t_err.join();
    proc.wait();

    if (ex_out) {
        std::rethrow_exception(ex_out);
    }
    if (ex_err) {
        std::rethrow_exception(ex_err);
    }
    return proc.exit_code();

#else // Windows branch
    const std::string certPath = getCertFile();
    if (!certPath.empty()) {
        _putenv_s("SSL_CERT_FILE", certPath.c_str());
    }
    // Lambda to quote arguments if they contain spaces or quotes
    auto quoteArg = [](const std::string &arg) -> std::string {
        if (arg.empty()) {
            return "\"\"";
        }
        bool needQuotes = arg.find_first_of(" \t\"") != std::string::npos;
        if (!needQuotes) {
            return arg;
        }

        std::string result = "\"";
        for (char c : arg) {
            if (c == '"') {
                result += "\\\""; // escape quotes
            } else {
                result += c;
            }
        }
        result += "\"";
        return result;
    };

    // Build command line safely
    std::ostringstream cmdLine;
    cmdLine << "cmd.exe /C " << quoteArg(command);
    for (const auto &a : args) {
        cmdLine << " " << quoteArg(a);
    }
    std::string cmdLineStr = cmdLine.str();

    // Create pipes
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE outRead, outWrite;
    HANDLE errRead, errWrite;
    if (!CreatePipe(&outRead, &outWrite, &sa, 0) || !CreatePipe(&errRead, &errWrite, &sa, 0)) {
        throw std::runtime_error("Failed to create pipes");
    }
    SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(errRead, HANDLE_FLAG_INHERIT, 0);

    // Setup startup info
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.hStdOutput = outWrite;
    si.hStdError = errWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi{};
    if (!CreateProcessA(nullptr, cmdLineStr.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi)) {
        CloseHandle(outRead);
        CloseHandle(outWrite);
        CloseHandle(errRead);
        CloseHandle(errWrite);
        throw std::runtime_error("CreateProcess failed");
    }

    CloseHandle(outWrite);
    CloseHandle(errWrite);

    // Lambda to read from a pipe and forward to print callback
    auto read_pipe = [this](HANDLE pipe) {
        char buffer[4096];
        DWORD n;
        while (ReadFile(pipe, buffer, sizeof(buffer) - 1, &n, nullptr) && n > 0) {
            buffer[n] = '\0';
            std::string_view sv(buffer, n);
            while (!sv.empty() && (sv.back() == '\n' || sv.back() == '\r')) {
                sv.remove_suffix(1);
            }
            if (!sv.empty()) {
                std::string s(sv);
                const size_t MAX = 1024; // safe size for Pd
                while (s.size() > MAX) {
                    this->print(s.substr(0, MAX), Pd4WebLogLevel::PD4WEB_LOG2);
                    s.erase(0, MAX);
                }
                if (!s.empty()) {
                    this->print(s, Pd4WebLogLevel::PD4WEB_LOG2);
                }
            }
        }
        CloseHandle(pipe);
    };

    // Start threads to read stdout and stderr concurrently
    std::thread t_out(read_pipe, outRead);
    std::thread t_err(read_pipe, errRead);

    WaitForSingleObject(pi.hProcess, INFINITE);
    t_out.join();
    t_err.join();

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exitCode);
#endif
}

// ─────────────────────────────────────
std::string Pd4Web::readFile(const std::string &path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ─────────────────────────────────────
void Pd4Web::writeFile(const std::string &path, const std::string &content) {
    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to write file: " + path);
    }
    file << content;
}

// ─────────────────────────────────────
void Pd4Web::replaceAll(std::string &str, const std::string &from, const std::string &to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

// ─────────────────────────────────────
std::string Pd4Web::formatLibUrl(const std::string &format, const std::string &arg1,
                                 const std::string &arg2) {
    std::string result = format;
    size_t pos = result.find("{}");
    if (pos != std::string::npos) {
        result.replace(pos, 2, arg1);
        pos = result.find("{}", pos + arg1.length());
        if (pos != std::string::npos) {
            result.replace(pos, 2, arg2);
        }
    }
    return result;
}

// ─────────────────────────────────────
bool Pd4Web::isNumber(const std::string &s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

// ──────────────────────────────────────────
void Pd4Web::print(std::string msg, enum Pd4WebLogLevel color, int level) {
    if (color == Pd4WebLogLevel::PD4WEB_ERROR) {
        m_Error = true;
    }

    if (m_PrintCallback) {
        m_PrintCallback(msg, color, level);
        return;
    }

    if (msg == "\n") {
        std::cout << std::endl;
        return;
    }

    std::string tablevel(level * 2, ' ');
    const std::string RESET = "\033[0m";

    switch (color) {
    case Pd4WebLogLevel::PD4WEB_ERROR: {
        std::cout << tablevel << "\033[31mERROR: " << msg << RESET << std::endl;
        if (m_FailFast) {
            throw std::runtime_error(msg);
        }
        break;
    }
    case Pd4WebLogLevel::PD4WEB_WARNING: {
        std::cout << tablevel << "\033[33mWARNING: " << msg << RESET << std::endl;
        break;
    }
    case Pd4WebLogLevel::PD4WEB_LOG1: {
        std::cout << tablevel << "\033[34m" << msg << RESET << std::endl;
        break;
    }
    case Pd4WebLogLevel::PD4WEB_LOG2: {
        std::cout << tablevel << "\033[32m" << msg << RESET << std::endl;
        break;
    }
    case Pd4WebLogLevel::PD4WEB_VERBOSE: {
        if (m_DevDebug) {
            std::cout << tablevel << msg << RESET << std::endl;
        }
        break;
    }
    }
}

// ──────────────────────────────────────────
void Pd4Web::serverPatch(bool toggle) {
    static std::unique_ptr<httplib::Server> server;

    if (toggle) {
        if (!server) {
            server = std::make_unique<httplib::Server>();
        }

        std::thread t([this]() {
            server->set_mount_point("/", m_BuildFolder);
            server->Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_redirect("/index.html");
            });
            server->Get("/stop",
                        [&](const httplib::Request &, httplib::Response &res) { server->stop(); });
            std::string site = "http://localhost:8082";
            print("Starting server at " + site, Pd4WebLogLevel::PD4WEB_LOG1);
            if (!server->listen("0.0.0.0", 8082)) {
                print("Failed to start server at " + site, Pd4WebLogLevel::PD4WEB_ERROR);
                m_Error = true;
                return;
            }
        });
        t.detach();
    } else {
        httplib::Client client("http://localhost:8082");
        auto res = client.Get("/stop");
        server.reset();
    }
}
