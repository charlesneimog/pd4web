#include "pd4web.hpp"

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
    if (s.empty())
        return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

// ──────────────────────────────────────────
void Pd4Web::print(std::string msg, enum Pd4WebColor color) {
    if (msg == "\n") {
        std::cout << std::endl;
        return;
    }

    const std::string RESET = "\033[0m";
    switch (color) {
    case Pd4WebColor::RED: {
        std::cout << "\033[31m    🔴️ ERROR: " << msg << RESET << std::endl;
        break;
    }
    case Pd4WebColor::YELLOW: {
        std::cout << "\033[33m    🟡️ WARNING: " << msg << RESET << std::endl;
        break;
    }
    case Pd4WebColor::GREEN: {
        std::cout << "\033[32m    🟢️ " << msg << RESET << std::endl;
        break;
    }
    case Pd4WebColor::BLUE: {
        std::cout << "\033[34m    🔵️ " << msg << RESET << std::endl;
        break;
    }
    }
}
