#include "pd4web_compiler.hpp"
#include <cstdlib>
#include <filesystem>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void run(const std::vector<std::string> &args) {
#if defined(__APPLE__) || defined(__linux__)
    std::string home = std::getenv("HOME");
    if (home.empty()) {
        printf("Failed to get home directory\n");
        exit(-1);
    }
    std::filesystem::path pd4webHome = std::filesystem::path(home) / ".local" / "share" / "pd4web";
    std::filesystem::create_directories(pd4webHome);
#else
    std::string appdata = std::getenv("APPDATA");
    std::filesystem::path pd4webHome = std::filesystem::path(appdata) / "pd4web";
    std::filesystem::create_directories(pd4webHome);
#endif

    Pd4Web pd4web(pd4webHome.string());

    // Convert vector<string> to argc/argv style
    int argc = static_cast<int>(args.size());
    std::vector<char *> argv(argc);
    for (int i = 0; i < argc; ++i) {
        argv[i] = const_cast<char *>(args[i].c_str());
    }

    pd4web.parseArgs(argc, argv.data());

    pd4web.init();
    pd4web.processPatch();
}

PYBIND11_MODULE(pypd4web, m) {
    m.doc() = "pd4web compiles PureData patches with external objects for Wasm, allowing to run "
              "entire patches in web browsers.";
    m.def("run", &run, py::arg("args"), "Run pd4web with command-line arguments");

    // Class
    py::class_<Pd4Web>(m, "Pd4Web")
        .def(py::init<const std::string &>())
        .def("parseArgs",
             [](Pd4Web &self, const std::vector<std::string> &args) {
                 int argc = static_cast<int>(args.size());
                 std::vector<char *> argv(argc);
                 for (int i = 0; i < argc; ++i) {
                     argv[i] = const_cast<char *>(args[i].c_str());
                 }
                 self.parseArgs(argc, argv.data());
             })
        .def("init", &Pd4Web::init)
        .def("processPatch", &Pd4Web::processPatch)

        // options
        .def("setPatchFile", &Pd4Web::setPatchFile)
        .def("setInitialMemory", &Pd4Web::setInitialMemory)
        .def("setPatchZoom", &Pd4Web::setPatchZoom)
        .def("setOutputFolder", &Pd4Web::setOutputFolder)
        .def("setPd4WebFilesFolder", &Pd4Web::setPd4WebFilesFolder)
        .def("setTemplateId", &Pd4Web::setTemplateId)
        .def("setDebugMode", &Pd4Web::setDebugMode)
        .def("setDevDebugMode", &Pd4Web::setDevDebugMode)
        .def("setFailFast", &Pd4Web::setFailFast)
        .def("disableGuiRender", &Pd4Web::disableGuiRender);
}
