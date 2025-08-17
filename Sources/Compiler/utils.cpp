#include "pd4web_compiler.hpp"

#include <fstream>

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
    case Pd4WebLogLevel::ERROR: {
        std::cout << tablevel << "\033[31mERROR: " << msg << RESET << std::endl;
        m_Error = true;
        if (m_FailFast) {
            throw std::runtime_error(msg);
        }
        break;
    }
    case Pd4WebLogLevel::WARNING: {
        std::cout << tablevel << "\033[33mWARNING: " << msg << RESET << std::endl;
        break;
    }
    case Pd4WebLogLevel::LOG1: {
        std::cout << tablevel << "\033[34m" << msg << RESET << std::endl;
        break;
    }
    case Pd4WebLogLevel::LOG2: {
        std::cout << tablevel << "\033[32m" << msg << RESET << std::endl;
        break;
    }
    case Pd4WebLogLevel::VERBOSE: {
        if (m_DevDebug) {
            std::cout << tablevel << msg << RESET << std::endl;
        }
        break;
    }
    }
}
