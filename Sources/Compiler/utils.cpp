#include "pd4web_compiler.hpp"

#define BOOST_PROCESS_VERSION 2
#include <fstream>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <iostream>

namespace bp = boost::process::v2; 
namespace asio = boost::asio;

// ─────────────────────────────────────
int Pd4Web::execProcess(const std::string& command, std::vector<std::string>& args) {
    {
        std::ostringstream oss;
        for (const auto& a : args) {
            oss << a << ' ';
        }
        print(command + " " + oss.str());
    }

    asio::io_context ctx;
    boost::asio::readable_pipe out{ctx};
    boost::asio::readable_pipe err{ctx};

#if defined(_WIN32)
    std::vector<std::string> fullArgs;
    fullArgs.push_back("/C");
    fullArgs.push_back(command);
    fullArgs.insert(fullArgs.end(), args.begin(), args.end());

    bp::process proc(
        ctx,
        "cmd.exe",
        fullArgs,
        bp::process_stdio{ .in = {}, .out = out, .err = err }
    );
#else
    bp::process proc(
        ctx,
        command,
        args,
        bp::process_stdio{ .in = {}, .out = out, .err = err }
    );
#endif

    auto read_loop = [&](boost::asio::readable_pipe& pipe, Pd4WebLogLevel level) {
        std::array<char, 4096> buf;
        for (;;) {
            boost::system::error_code ec;
            std::size_t n = pipe.read_some(boost::asio::buffer(buf), ec);
            if (ec == boost::asio::error::eof) break;
            if (ec) throw boost::system::system_error(ec);
            if (n == 0) continue;

            std::string_view sv(buf.data(), n);
            if (sv.back() == '\n') {
                sv.remove_suffix(1);
            }
            if (sv.empty()) continue;

            print(std::string(sv), level);
        }
    };

    std::exception_ptr ex_out, ex_err;
    std::thread t_out([&] {
        try { read_loop(out, Pd4WebLogLevel::PD4WEB_LOG2); }
        catch (...) { ex_out = std::current_exception(); }
    });
    std::thread t_err([&] {
        try { read_loop(err, Pd4WebLogLevel::PD4WEB_LOG2); }
        catch (...) { ex_err = std::current_exception(); }
    });

    proc.wait();
    t_out.join();
    t_err.join();

    if (ex_out) std::rethrow_exception(ex_out);
    if (ex_err) std::rethrow_exception(ex_err);

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
            server->Get("/stop", [&](const httplib::Request &, httplib::Response &res) {
                server->stop();
            });
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
