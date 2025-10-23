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
#endif

namespace bp = boost::process::v2;
namespace asio = boost::asio;

// ──────────────────────────────────────────
#if defined(_WIN32)
namespace {
bool appendWindowsStoreToPem(const wchar_t *storeName, DWORD storeLocation, std::ofstream &out) {
    HCERTSTORE store = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W,
        0,
        static_cast<HCRYPTPROV_LEGACY>(NULL),
        storeLocation | CERT_STORE_READONLY_FLAG,
        storeName);

    if (!store) {
        return false;
    }

    PCCERT_CONTEXT context = nullptr;
    bool wroteCertificates = false;
    while ((context = CertEnumCertificatesInStore(store, context)) != nullptr) {
        DWORD requiredSize = 0;
        if (!CryptBinaryToStringA(context->pbCertEncoded, context->cbCertEncoded,
                                  CRYPT_STRING_BASE64HEADER | CRYPT_STRING_NOCRLF, nullptr,
                                  &requiredSize)) {
            continue;
        }

        std::string pem(requiredSize, '\0');
        if (!CryptBinaryToStringA(context->pbCertEncoded, context->cbCertEncoded,
                                  CRYPT_STRING_BASE64HEADER | CRYPT_STRING_NOCRLF, pem.data(),
                                  &requiredSize)) {
            continue;
        }

        if (!pem.empty() && pem.back() == '\0') {
            pem.pop_back();
        }

        out << pem << "\n";
        wroteCertificates = true;
    }

    CertCloseStore(store, 0);
    return wroteCertificates;
}

bool exportWindowsRootCertificates(const std::filesystem::path &targetPath) {
    std::ofstream out(targetPath, std::ios::binary);
    if (!out) {
        return false;
    }

    bool wroteAny = false;
    wroteAny |= appendWindowsStoreToPem(L"ROOT", CERT_SYSTEM_STORE_CURRENT_USER, out);
    wroteAny |= appendWindowsStoreToPem(L"ROOT", CERT_SYSTEM_STORE_LOCAL_MACHINE, out);

    out.flush();
    return wroteAny;
}
} // namespace
#endif

std::string Pd4Web::getCertFile() {
    namespace fs = std::filesystem;

    auto envCertPath = [&](const char *name) -> std::string {
        if (const char *value = std::getenv(name)) {
            std::error_code ec;
            if (fs::exists(value, ec)) {
                return value;
            }
        }
        return "";
    };

    if (auto env = envCertPath("SSL_CERT_FILE"); !env.empty()) {
        return env;
    }
    if (auto env = envCertPath("REQUESTS_CA_BUNDLE"); !env.empty()) {
        return env;
    }
    if (auto env = envCertPath("CURL_CA_BUNDLE"); !env.empty()) {
        return env;
    }

#if defined(_WIN32)
    fs::path certDir = fs::path(m_Pd4WebRoot) / "certificates";
    std::error_code ec;
    fs::create_directories(certDir, ec);

    fs::path certFile = certDir / "windows-root.pem";
    auto fileSize = fs::file_size(certFile, ec);
    if (ec || fileSize == 0) {
        if (!exportWindowsRootCertificates(certFile)) {
            print("Unable to export Windows root certificates. TLS downloads may fail.",
                  Pd4WebLogLevel::PD4WEB_WARNING);
            return "";
        }
    }
    return certFile.string();
#else
    static const std::vector<std::string> cafiles = {
        "/etc/ssl/certs/ca-certificates.crt",
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/ssl/cert.pem",
        "/etc/ssl/certs/ca-bundle.crt"};

    for (const auto &candidate : cafiles) {
        std::error_code ecCandidate;
        if (fs::exists(candidate, ecCandidate)) {
            return candidate;
        }
    }

    return "";
#endif
}

// ─────────────────────────────────────
int Pd4Web::execProcess(const std::string &command, std::vector<std::string> &args) {
    {
        std::ostringstream oss;
        for (const auto &a : args) {
            oss << a << ' ';
        }
        print(command + " " + oss.str());
    }

    std::string certPath = getCertFile();
    if (!certPath.empty()) {
        auto ensureEnv = [&](const char *name) {
            if (const char *current = std::getenv(name)) {
                std::error_code ec;
                if (std::filesystem::exists(current, ec)) {
                    return;
                }
            }
#if defined(_WIN32)
            _putenv_s(name, certPath.c_str());
#else
            setenv(name, certPath.c_str(), 1);
#endif
        };

        ensureEnv("SSL_CERT_FILE");
        ensureEnv("REQUESTS_CA_BUNDLE");
        ensureEnv("CURL_CA_BUNDLE");
    }

    asio::io_context ctx;
    boost::asio::readable_pipe out{ctx};
    boost::asio::readable_pipe err{ctx};

#if defined(_WIN32)
    std::vector<std::string> fullArgs;
    fullArgs.push_back("/C");
    fullArgs.push_back(command);
    fullArgs.insert(fullArgs.end(), args.begin(), args.end());

    bp::process proc(ctx, "cmd.exe", fullArgs, bp::process_stdio{.in = {}, .out = out, .err = err});
#else
    bp::process proc(ctx, command, args, bp::process_stdio{.in = {}, .out = out, .err = err});
#endif

    auto read_loop = [&](boost::asio::readable_pipe &pipe, Pd4WebLogLevel level) {
        std::array<char, 4096> buf;
        for (;;) {
            boost::system::error_code ec;
            std::size_t n = pipe.read_some(boost::asio::buffer(buf), ec);
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
            if (sv.back() == '\n') {
                sv.remove_suffix(1);
            }
            if (sv.empty()) {
                continue;
            }

            print(std::string(sv), level);
        }
    };

    std::exception_ptr ex_out, ex_err;
    std::thread t_out([&] {
        try {
            read_loop(out, Pd4WebLogLevel::PD4WEB_LOG2);
        } catch (...) {
            ex_out = std::current_exception();
        }
    });
    std::thread t_err([&] {
        try {
            read_loop(err, Pd4WebLogLevel::PD4WEB_LOG2);
        } catch (...) {
            ex_err = std::current_exception();
        }
    });

    proc.wait();
    t_out.join();
    t_err.join();

    if (ex_out) {
        std::rethrow_exception(ex_out);
    }
    if (ex_err) {
        std::rethrow_exception(ex_err);
    }

    return proc.exit_code();
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
            server->set_mount_point("/", m_OutputFolder);
            server->Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_redirect("/index.html");
            });
            server->Get("/stop",
                        [&](const httplib::Request &, httplib::Response &res) { server->stop(); });
            std::string site = "http://localhost:8080";
            print("Starting server at " + site, Pd4WebLogLevel::PD4WEB_LOG1);
            if (!server->listen("0.0.0.0", 8080)) {
                print("Failed to start server at " + site, Pd4WebLogLevel::PD4WEB_ERROR);
                m_Error = true;
                return;
            }
        });
        t.detach();
    } else {
        httplib::Client client("http://localhost:8080");
        auto res = client.Get("/stop");
        server.reset(); // libera a memória
    }
}
