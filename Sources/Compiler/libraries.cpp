#include "pd4web_compiler.hpp"

#include <fstream>
#include <stack>

// ─────────────────────────────────────
bool Pd4Web::libIsSupported(std::string libName) {
    PD4WEB_LOGGER();

    for (Library lib : m_Libraries) {
        if (lib.Name == libName) {
            fs::path completPath = fs::path(m_Pd4WebRoot) / libName;
            if (!fs::exists(completPath)) {
                bool ok = gitClone(lib.Url, libName, lib.Version);
                if (!ok) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::downloadSupportedLib(std::string libName) {
    PD4WEB_LOGGER();
    for (Library Lib : m_Libraries) {
        if (Lib.Name == libName) {
            bool ok = gitClone(Lib.Url, libName, Lib.Version);
            if (!ok) {
                print("Failed to clone library'" + libName + "'", Pd4WebLogLevel::PD4WEB_ERROR);
                return false;
            }
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::getSupportedLibraries(std::shared_ptr<Patch> &Patch) {
    PD4WEB_LOGGER();
    m_Libraries.clear();

    std::ifstream file(Patch->Pd4WebFiles / "Libraries" / "Libraries.yaml");

    if (!file) {
        print("Failed to open libraries file", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    YamlNode node = fkyaml::node::deserialize(file);
    if (!node.contains("Libraries")) {
        print("YAML does not have libraries", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    m_LibrariesNode = node.at("Libraries");
    m_SourcesNode = node.at("Sources");

    if (!m_LibrariesNode.is_sequence()) {
        print("YAML file has no sequence", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    std::unordered_set<std::string> supportedLibraries;
    for (int i = 0; i < m_LibrariesNode.size(); i++) {
        YamlNode libNode = m_LibrariesNode.at(i);
        std::string lib = libNode["Name"].get_value<std::string>();
        supportedLibraries.insert(lib);

        Library newLib;
        newLib.Name = lib;
        newLib.Source = libNode["Source"].get_value<std::string>();
        newLib.Developer = libNode["Developer"].get_value<std::string>();
        newLib.Repository = libNode["Repository"].get_value<std::string>();
        newLib.Version = libNode["Version"].get_value<std::string>();
        if (libNode.contains("RunConfigure")) {
            newLib.ConfigureBeforeCheckObjects = libNode["RunConfigure"].get_value<bool>();
        } else {
            newLib.ConfigureBeforeCheckObjects = false;
        }

        std::string url = m_SourcesNode[newLib.Source].get_value<std::string>();
        std::string urlGit = formatLibUrl(url, newLib.Developer, newLib.Repository);
        urlGit += ".git";
        newLib.Url = urlGit;

        m_Libraries.push_back(newLib);
    }

    for (const std::string &declared : Patch->DeclaredLibs) {
        if (supportedLibraries.find(declared) == supportedLibraries.end()) {
            print("Library '" + declared + "' is not supported", Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        } else {
            for (Library Lib : m_Libraries) {
                if (Lib.Name == declared) {
                    bool ok = gitClone(Lib.Url, Lib.Name, Lib.Version);
                    if (!ok) {
                        print("Failed to clone library'" + declared + "'",
                              Pd4WebLogLevel::PD4WEB_ERROR);
                        return false;
                    }
                    return true;
                }
            }
        }
    }

    return true;
}

// ─────────────────────────────────────
bool Pd4Web::configureLibraryForObjectScan(std::shared_ptr<Patch> &p, const std::string &Lib) {
    auto library = std::find_if(m_Libraries.begin(), m_Libraries.end(),
                                [&](const Library &entry) { return entry.Name == Lib; });
    if (library == m_Libraries.end() || !library->ConfigureBeforeCheckObjects ||
        m_ConfiguredLibraryScans.contains(Lib)) {
        return true;
    }

    fs::path sourceDir = p->Pd4WebRoot / Lib;
    fs::path buildDir = sourceDir / ".build";
    if (!fs::is_directory(sourceDir)) {
        return false;
    }

    std::vector<std::string> cfgArgs = {
        m_Cmake.string(),
        sourceDir.string(),
        "-B",
        buildDir.string(),
        "-G",
        "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_MAKE_PROGRAM=" + m_Ninja.string(),
        "-DCPM_SOURCE_CACHE=" + (p->Pd4WebRoot / "Dependencies").string(),
        "-DPD4WEB_AS_ES6=" + std::to_string(m_ExportES6Module),
        "-Wno-dev",
    };
    int result = execProcess(m_Emcmake.string(), cfgArgs);
    if (result != 0) {
        print("Configuration step failed while discovering dependencies of library '" + Lib + "'",
              Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    m_ConfiguredLibraryScans.insert(Lib);
    m_LibrarySearchRoots.erase(Lib);
    m_IndexedLuaLibraries.erase(Lib);
    m_DependencyLuaObjects.clear();
    m_DependencyLuaIndexBuilt = false;
    return true;
}

// ─────────────────────────────────────
std::vector<fs::path> Pd4Web::getLibrarySearchRoots(std::shared_ptr<Patch> &p,
                                                    const std::string &Lib) {
    if (Lib.empty()) {
        return {};
    }
    if (auto cached = m_LibrarySearchRoots.find(Lib); cached != m_LibrarySearchRoots.end()) {
        return cached->second;
    }

    (void)configureLibraryForObjectScan(p, Lib);

    std::vector<fs::path> roots;
    std::unordered_set<std::string> known;
    auto addRoot = [&](const fs::path &root) {
        std::error_code ec;
        if (!fs::is_directory(root, ec)) {
            return;
        }
        fs::path normalized = fs::weakly_canonical(root, ec);
        if (ec) {
            normalized = root.lexically_normal();
        }
        if (known.insert(normalized.generic_string()).second) {
            roots.push_back(normalized);
        }
    };

    fs::path libraryRoot = p->Pd4WebRoot / Lib;
    addRoot(libraryRoot);

    // CMake/CPM materializes every dependency owned by this library beneath
    // <library>/.build/_deps/<dependency>-src. This gives us an unambiguous
    // library-to-dependency relationship without guessing from the global cache.
    fs::path dependencyBuildRoot = libraryRoot / ".build" / "_deps";
    std::error_code ec;
    fs::directory_iterator dependencies(dependencyBuildRoot,
                                        fs::directory_options::skip_permission_denied, ec);
    fs::directory_iterator end;
    while (!ec && dependencies != end) {
        const fs::path path = dependencies->path();
        const std::string name = path.filename().string();
        if (dependencies->is_directory(ec) && name.ends_with("-src")) {
            addRoot(path);
        }
        dependencies.increment(ec);
    }

    m_LibrarySearchRoots[Lib] = roots;
    return roots;
}

// ─────────────────────────────────────
std::vector<fs::path> Pd4Web::findDependencyLuaObjects(std::shared_ptr<Patch> &p,
                                                       const std::string &objectName) {
    if (!m_DependencyLuaIndexBuilt) {
        fs::path dependenciesRoot = p->Pd4WebRoot / "Dependencies";
        std::error_code ec;
        fs::recursive_directory_iterator it(dependenciesRoot,
                                            fs::directory_options::skip_permission_denied, ec);
        fs::recursive_directory_iterator end;
        while (!ec && it != end) {
            if (it->is_directory(ec)) {
                const std::string name = it->path().filename().string();
                if (name == ".git" || name == ".build") {
                    it.disable_recursion_pending();
                }
            } else if (it->is_regular_file(ec) && it->path().extension() == ".pd_lua") {
                const std::string objectName = it->path().stem().string();
                fs::path normalized = fs::weakly_canonical(it->path(), ec);
                if (ec) {
                    normalized = it->path().lexically_normal();
                    ec.clear();
                }
                auto &objects = m_DependencyLuaObjects[objectName];
                if (std::find(objects.begin(), objects.end(), normalized) == objects.end()) {
                    objects.push_back(normalized);
                }
            }
            it.increment(ec);
        }
        m_DependencyLuaIndexBuilt = true;
    }

    auto found = m_DependencyLuaObjects.find(objectName);
    return found == m_DependencyLuaObjects.end() ? std::vector<fs::path>{} : found->second;
}

// ─────────────────────────────────────
std::vector<fs::path> Pd4Web::findLuaObjects(std::shared_ptr<Patch> &Patch, fs::path dir,
                                             PatchLine &pl) {
    PD4WEB_LOGGER();
    std::vector<fs::path> results;
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        return results;
    }

    if (std::find(Patch->PdLuaFolderSearch.begin(), Patch->PdLuaFolderSearch.end(), dir) !=
        Patch->PdLuaFolderSearch.end()) {
        for (std::string objName : Patch->ValidLuaObjects) {
            if (objName == pl.Name) {
                pl.Found = true;
                pl.isLuaExternal = true;
            }
        }
        return results;
    }

    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".pd_lua") {
            results.push_back(entry.path());
            std::string objName = entry.path().stem().string();
            if (objName == pl.Name) {
                pl.Found = true;
                pl.isLuaExternal = true;
            }
            Patch->ValidLuaObjects.push_back(objName);
        }
    }
    Patch->PdLuaFolderSearch.push_back(dir);

    return results;
}

// ─────────────────────────────────────
std::vector<std::string> Pd4Web::GetLuaRequireFolder(const std::string &filePath) {
    std::vector<std::string> lua_requires;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return lua_requires;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Lua accepts both require("module") and require "module". Keep this parser
    // deliberately small: dependency resolution only needs static string modules.
    std::regex re(R"lua(\brequire\s*(?:\(\s*)?["']([^"']+)["']\s*\)?)lua");
    std::smatch match;

    auto begin = content.cbegin();
    auto end = content.cend();

    while (std::regex_search(begin, end, match, re)) {
        lua_requires.push_back(match[1]);
        begin = match.suffix().first;
    }

    return lua_requires;
}

// ─────────────────────────────────────
void Pd4Web::loadObjectsJson(std::shared_ptr<Patch> &p) {
    std::vector<std::string> absNames;
    const std::string jsonFile = (p->Pd4WebRoot / "objects.json").string();
    json full_json;
    std::ifstream in(jsonFile);
    if (in.is_open()) {
        in >> full_json;
        in.close();
        p->ExternalObjectsJson = full_json;
    } else {
        print("Failed to load list of objects from libraries", Pd4WebLogLevel::PD4WEB_ERROR);
    }
}

// ─────────────────────────────────────
std::vector<std::string> Pd4Web::listLuaObjectsInLibrary(std::shared_ptr<Patch> &p,
                                                         std::string Lib) {
    PD4WEB_LOGGER();
    if (Lib.empty()) {
        return {};
    }

    const fs::path jsonPath = p->Pd4WebRoot / "objects.json";
    json full_json;
    std::ifstream in(jsonPath);
    if (in.is_open()) {
        try {
            in >> full_json;
        } catch (const json::exception &error) {
            print("Failed to parse objects.json while listing Lua objects: " +
                      std::string(error.what()),
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return {};
        }
    }

    if (m_IndexedLuaLibraries.contains(Lib) && full_json.contains(Lib) &&
        full_json[Lib].contains("lua_objects")) {
        std::vector<std::string> cachedObjects;
        for (auto object = full_json[Lib]["lua_objects"].begin();
             object != full_json[Lib]["lua_objects"].end(); ++object) {
            cachedObjects.push_back(object.key());
        }
        p->ExternalObjectsJson = full_json;
        return cachedObjects;
    }

    std::vector<fs::path> roots = getLibrarySearchRoots(p, Lib);
    if (roots.empty()) {
        return {};
    }

    full_json[Lib]["lua_objects"] = json::object();
    full_json[Lib]["dependencies"] = json::object();
    std::vector<std::string> objectNames;
    std::unordered_set<std::string> knownObjects;
    const fs::path libraryRoot = p->Pd4WebRoot / Lib;

    for (const fs::path &root : roots) {
        if (root != libraryRoot) {
            std::string dependencyName = root.filename().string();
            if (dependencyName.ends_with("-src")) {
                dependencyName.resize(dependencyName.size() - 4);
            }
            full_json[Lib]["dependencies"][dependencyName] = root.string();
        }

        std::error_code ec;
        fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied,
                                            ec);
        fs::recursive_directory_iterator end;
        while (!ec && it != end) {
            if (it->is_directory(ec)) {
                const std::string directoryName = it->path().filename().string();
                if (directoryName == ".git" || directoryName == ".build") {
                    it.disable_recursion_pending();
                }
            } else if (it->is_regular_file(ec) && it->path().extension() == ".pd_lua") {
                const std::string name = it->path().stem().string();
                if (knownObjects.insert(name).second) {
                    objectNames.push_back(name);
                    full_json[Lib]["lua_objects"][name] = {name, it->path().string()};
                }
            }
            it.increment(ec);
        }
    }

    std::ofstream out(jsonPath);
    if (!out) {
        print("Unable to update Lua objects in " + jsonPath.string(), Pd4WebLogLevel::PD4WEB_ERROR);
        return {};
    }
    out << full_json.dump(2);
    p->ExternalObjectsJson = full_json;
    m_IndexedLuaLibraries.insert(Lib);

    print("Found " + std::to_string(objectNames.size()) + " Lua objects owned by library '" + Lib +
              "' and its dependencies",
          Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
    return objectNames;
}

// ─────────────────────────────────────
std::vector<std::string> Pd4Web::listAbstractionsInLibrary(std::shared_ptr<Patch> &p,
                                                           std::string Lib) {
    PD4WEB_LOGGER();

    std::vector<std::string> absNames;
    const std::string jsonFile = (p->Pd4WebRoot / "objects.json").string();
    json full_json;
    std::ifstream in(jsonFile);
    if (in.is_open()) {
        in >> full_json;
        in.close();
        if (full_json.contains(Lib)) {
            if (full_json[Lib].contains("abstractions")) {
                print("Found abstractions in library '" + Lib + "'", Pd4WebLogLevel::PD4WEB_LOG2,
                      p->printLevel + 1);
                const auto &libEntry = full_json[Lib]["abstractions"];
                std::vector<std::string> keys;
                for (auto it = libEntry.begin(); it != libEntry.end(); ++it) {
                    keys.push_back(it.key());
                }
                p->ExternalObjectsJson = full_json;
                return keys;
            }
        }
    }

    fs::path completPath = p->Pd4WebRoot / Lib;
    print(completPath.string(), Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
    if (!fs::exists(completPath)) {
        completPath = p->Pd4WebRoot / Lib;
        if (!fs::exists(completPath)) {
            print("Library '" + Lib + "' not found in Pd4Web root", Pd4WebLogLevel::PD4WEB_ERROR);
            std::vector<std::string> patchNames;
            return patchNames;
        }
    }

    print("Listing all Abstractions inside '" + Lib +
              "'. This is done once for library and will take a while. Please wait...",
          Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    std::vector<std::string> patchNames;
    std::vector<std::string> patchPath;

    for (const auto &entry : fs::recursive_directory_iterator(completPath)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".pd") {
            std::string name = entry.path().stem().string();
            std::string path = entry.path().string();
            patchNames.push_back(name);
            patchPath.push_back(path);
        }
    }

    for (size_t i = 0; i < patchNames.size(); ++i) {
        full_json[Lib]["abstractions"][patchNames[i]] = {patchNames[i], patchPath[i]};
    }

    std::ofstream out(jsonFile);
    out << full_json.dump(2);
    out.close();
    p->ExternalObjectsJson = full_json;

    return patchNames;
}

// ─────────────────────────────────────
void treesitterCheckForSetupFunction(std::string &content, TSNode node,
                                     std::vector<std::string> &objectNames,
                                     std::vector<std::string> &setupNames,
                                     std::vector<std::string> &setupSignatures) {
    PD4WEB_LOGGER();
    if (ts_node_is_null(node)) {
        return;
    }

    if (strcmp(ts_node_type(node), "call_expression") == 0) {
        TSNode func_node = ts_node_child_by_field_name(node, "function", 8);
        if (ts_node_is_null(func_node)) {
            return;
        }

        std::string func_text(content.data() + ts_node_start_byte(func_node),
                              ts_node_end_byte(func_node) - ts_node_start_byte(func_node));

        bool is_class_new = func_text.find("class_new") != std::string::npos;
        bool is_class_addcreator = func_text.find("class_addcreator") != std::string::npos;
        if (!is_class_new && !is_class_addcreator) {
            return;
        }

        TSNode args_node = ts_node_child_by_field_name(node, "arguments", 9);
        if (ts_node_is_null(args_node)) {
            return;
        }

        uint32_t args_count = ts_node_named_child_count(args_node);
        uint32_t target_arg_index = is_class_new ? 0 : 1;
        if (args_count <= target_arg_index) {
            return;
        }

        TSNode target_arg = ts_node_named_child(args_node, target_arg_index);
        if (strcmp(ts_node_type(target_arg), "call_expression") != 0) {
            return;
        }

        TSNode inner_func = ts_node_child_by_field_name(target_arg, "function", 8);
        if (ts_node_is_null(inner_func)) {
            return;
        }

        std::string inner_func_text(content.data() + ts_node_start_byte(inner_func),
                                    ts_node_end_byte(inner_func) - ts_node_start_byte(inner_func));
        if (inner_func_text != "gensym") {
            return;
        }

        TSNode gensym_args = ts_node_child_by_field_name(target_arg, "arguments", 9);
        if (ts_node_named_child_count(gensym_args) < 1) {
            return;
        }

        TSNode string_arg = ts_node_named_child(gensym_args, 0);
        if (strcmp(ts_node_type(string_arg), "string_literal") != 0) {
            return;
        }

        std::string object_name(content.data() + ts_node_start_byte(string_arg) + 1,
                                ts_node_end_byte(string_arg) - ts_node_start_byte(string_arg) - 2);

        if (std::find(objectNames.begin(), objectNames.end(), object_name) != objectNames.end()) {
            return;
        }

        objectNames.push_back(object_name);

        // ← Find function_definition parent
        TSNode current = node;
        while (!ts_node_is_null(current)) {
            if (strcmp(ts_node_type(current), "function_definition") == 0) {
                break;
            }
            current = ts_node_parent(current);
        }

        if (ts_node_is_null(current)) {
            return;
        }

        // ← Try extract setup function name
        std::string func_name;
        TSNode declarator =
            ts_node_child_by_field_name(current, "declarator", strlen("declarator"));

        // Try to find the declarator inside the function_definition
        if (!ts_node_is_null(declarator)) {
            TSNode identifier =
                ts_node_child_by_field_name(declarator, "identifier", strlen("identifier"));
            if (!ts_node_is_null(identifier)) {
                uint32_t id_start = ts_node_start_byte(identifier);
                uint32_t id_end = ts_node_end_byte(identifier);
                func_name = std::string(content.data() + id_start, id_end - id_start);
            } else {
                // Fallback 2: find first child named "identifier" under declarator
                uint32_t child_count = ts_node_named_child_count(declarator);
                for (uint32_t i = 0; i < child_count; ++i) {
                    TSNode child = ts_node_named_child(declarator, i);
                    if (strcmp(ts_node_type(child), "identifier") == 0) {
                        uint32_t id_start = ts_node_start_byte(child);
                        uint32_t id_end = ts_node_end_byte(child);
                        func_name = std::string(content.data() + id_start, id_end - id_start);
                        break;
                    }
                }
            }
        }

        // ← Extract full function signature
        uint32_t start = ts_node_start_byte(current);
        uint32_t end = ts_node_end_byte(current);
        std::string func_code(content.data() + start, end - start);
        size_t pos = func_code.find('{');
        std::string signature = (pos != std::string::npos) ? func_code.substr(0, pos) : func_code;
        signature.erase(signature.find_last_not_of(" \n\r\t") + 1);
        signature.erase(std::remove(signature.begin(), signature.end(), '\n'), signature.end());
        signature.erase(std::remove(signature.begin(), signature.end(), '\r'), signature.end());

        setupSignatures.push_back(signature);
        setupNames.push_back(func_name);
    }

    uint32_t child_count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        treesitterCheckForSetupFunction(content, ts_node_named_child(node, i), objectNames,
                                        setupNames, setupSignatures);
    }
}

// ─────────────────────────────────────
void Pd4Web::processCallExpression(std::string &content, TSNode node,
                                   std::vector<std::string> &objectNames,
                                   std::vector<std::string> &setupNames,
                                   std::vector<std::string> &setupSignatures) {
    TSNode func_node = ts_node_child_by_field_name(node, "function", 8);
    if (ts_node_is_null(func_node)) {
        return;
    }

    std::string func_text(content.data() + ts_node_start_byte(func_node),
                          ts_node_end_byte(func_node) - ts_node_start_byte(func_node));

    bool is_class_new = func_text.find("class_new") != std::string::npos;
    bool is_class_addcreator = func_text.find("class_addcreator") != std::string::npos;
    if (!is_class_new && !is_class_addcreator) {
        return;
    }

    TSNode args_node = ts_node_child_by_field_name(node, "arguments", 9);
    if (ts_node_is_null(args_node)) {
        return;
    }

    uint32_t args_count = ts_node_named_child_count(args_node);
    uint32_t target_arg_index = is_class_new ? 0 : 1;
    if (args_count <= target_arg_index) {
        return;
    }

    TSNode target_arg = ts_node_named_child(args_node, target_arg_index);
    if (strcmp(ts_node_type(target_arg), "call_expression") != 0) {
        return;
    }

    TSNode inner_func = ts_node_child_by_field_name(target_arg, "function", 8);
    if (ts_node_is_null(inner_func)) {
        return;
    }

    std::string inner_func_text(content.data() + ts_node_start_byte(inner_func),
                                ts_node_end_byte(inner_func) - ts_node_start_byte(inner_func));
    if (inner_func_text != "gensym") {
        return;
    }

    TSNode gensym_args = ts_node_child_by_field_name(target_arg, "arguments", 9);
    if (ts_node_named_child_count(gensym_args) < 1) {
        return;
    }

    TSNode string_arg = ts_node_named_child(gensym_args, 0);
    if (strcmp(ts_node_type(string_arg), "string_literal") != 0) {
        return;
    }

    std::string object_name(content.data() + ts_node_start_byte(string_arg) + 1,
                            ts_node_end_byte(string_arg) - ts_node_start_byte(string_arg) - 2);

    if (std::find(objectNames.begin(), objectNames.end(), object_name) != objectNames.end()) {
        return;
    }
    objectNames.push_back(object_name);

    // Find parent function_definition
    TSNode current = node;
    while (!ts_node_is_null(current)) {
        if (strcmp(ts_node_type(current), "function_definition") == 0) {
            break;
        }
        current = ts_node_parent(current);
    }
    if (ts_node_is_null(current)) {
        return;
    }

    // Extract function name
    std::string func_name;
    TSNode declarator = ts_node_child_by_field_name(current, "declarator", strlen("declarator"));
    if (!ts_node_is_null(declarator)) {
        TSNode identifier =
            ts_node_child_by_field_name(declarator, "identifier", strlen("identifier"));
        if (!ts_node_is_null(identifier)) {
            func_name.assign(content.data() + ts_node_start_byte(identifier),
                             ts_node_end_byte(identifier) - ts_node_start_byte(identifier));
        } else {
            uint32_t child_count = ts_node_named_child_count(declarator);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_named_child(declarator, i);
                if (strcmp(ts_node_type(child), "identifier") == 0) {
                    func_name.assign(content.data() + ts_node_start_byte(child),
                                     ts_node_end_byte(child) - ts_node_start_byte(child));
                    break;
                }
            }
        }
    }

    // Extract full signature
    uint32_t start = ts_node_start_byte(current);
    uint32_t end = ts_node_end_byte(current);
    std::string func_code(content.data() + start, end - start);
    size_t pos = func_code.find('{');
    std::string signature = (pos != std::string::npos) ? func_code.substr(0, pos) : func_code;
    signature.erase(signature.find_last_not_of(" \n\r\t") + 1);
    signature.erase(std::remove(signature.begin(), signature.end(), '\n'), signature.end());
    signature.erase(std::remove(signature.begin(), signature.end(), '\r'), signature.end());

    setupSignatures.push_back(signature);
    setupNames.push_back(func_name);
    print("Found setup function: " + func_name + " for object: " + object_name,
          Pd4WebLogLevel::PD4WEB_LOG2, 2);
}

// ─────────────────────────────────────
void Pd4Web::treesitterCheckForSetupFunction(std::string &content, TSNode root,
                                             std::vector<std::string> &objectNames,
                                             std::vector<std::string> &setupNames,
                                             std::vector<std::string> &setupSignatures) {
    PD4WEB_LOGGER();

    std::stack<TSNode> stack;
    stack.push(root);

    while (!stack.empty()) {
        TSNode node = stack.top();
        stack.pop();

        if (ts_node_is_null(node)) {
            continue;
        }

        if (strcmp(ts_node_type(node), "call_expression") == 0) {
            processCallExpression(content, node, objectNames, setupNames, setupSignatures);
        }

        uint32_t child_count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            stack.push(ts_node_named_child(node, i));
        }
    }
}

// ─────────────────────────────────────
std::vector<std::string> Pd4Web::listObjectsInLibrary(std::shared_ptr<Patch> &p, std::string Lib) {
    PD4WEB_LOGGER();
    std::vector<std::string> objectNames;
    std::vector<std::string> setupSignatures;
    std::vector<std::string> setupNames;

    if (libIsSupported(Lib)) {
        bool ok = downloadSupportedLib(Lib);
        if (!ok) {
            print("Error when trying to donwload library '" + Lib + "'.",
                  Pd4WebLogLevel::PD4WEB_ERROR);
        }
    }

    // Use filesystem::path to join paths reliably
    fs::path completPath = p->Pd4WebRoot / Lib;
    if (!fs::exists(completPath) || !fs::is_directory(completPath)) {
        if (libIsSupported(Lib)) {
            if (!downloadSupportedLib(Lib)) {
                print("Error to download Library'" + Lib + ".'", Pd4WebLogLevel::PD4WEB_ERROR);
                return objectNames;
            };
            if (!fs::exists(completPath) || !fs::is_directory(completPath)) {
                print("Library '" + Lib + "' not found", Pd4WebLogLevel::PD4WEB_ERROR);
                return objectNames;
            }
        } else {
            print("Library '" + Lib + "' not supported", Pd4WebLogLevel::PD4WEB_ERROR);
            return objectNames;
        }
    }

    // Configuring the owning library materializes its dependency graph under
    // <library>/.build/_deps. The main recursive scan below then includes both
    // the library and its dependency-provided Pd externals.
    std::vector<fs::path> librarySearchRoots = getLibrarySearchRoots(p, Lib);

    const fs::path jsonPath = p->Pd4WebRoot / "objects.json";
    const std::string jsonFile = jsonPath.string();

    json full_json;
    // Guard JSON read with try/catch to avoid exceptions turning into crashes
    try {
        std::ifstream in(jsonFile);
        if (in.is_open()) {
            in >> full_json;
            in.close();
            if (full_json.contains(Lib) && full_json[Lib].contains("objects") &&
                full_json[Lib].value("dependency_objects_indexed", false)) {
                const auto &libEntry = full_json[Lib]["objects"];
                std::vector<std::string> keys;
                for (auto it = libEntry.begin(); it != libEntry.end(); ++it) {
                    keys.push_back(it.key());
                }
                p->ExternalObjectsJson = full_json;
                return keys;
            }
        }
    } catch (const std::exception &e) {
        print(std::string("Failed reading objects.json: ") + e.what(),
              Pd4WebLogLevel::PD4WEB_ERROR);
    }

    print("Listing all Objects inside '" + Lib +
              "'. This is done once for library and will take a LONG time. Please wait...",
          Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);

    static int count = 0;

    for (const auto &entry : fs::recursive_directory_iterator(completPath)) {
        // Skip non-regular files early (symlinks / directories)
        count++;
        if (count % 100 == 0) {
            print("Processed " + std::to_string(count) + " files in library '" + Lib + "'",
                  Pd4WebLogLevel::PD4WEB_LOG2, p->printLevel + 1);
        }

        if (!entry.is_regular_file()) {
            continue;
        }

        // Skip git submodules (keeps behaviour from original)
        if (isFileFromGitSubmodule(completPath.string(), entry.path())) {
            continue;
        }

        fs::path pth = entry.path();
        std::string ext = pth.extension().string();

        TSParser *parser = nullptr;
        if (ext == ".c") {
            parser = m_cParser;
        } else if (ext == ".cpp") {
            parser = m_cppParser;
        } else {
            continue;
        }

        // Ensure we have a valid parser
        if (parser == nullptr) {
            // If parser isn't initialized skip this file and log once (optionally)
            print("No parser available for file " + pth.string(), Pd4WebLogLevel::PD4WEB_LOG1);
            continue;
        }

        std::ifstream inFile(pth, std::ios::binary);
        if (!inFile) {
            continue;
        }

        std::string content;
        content.assign((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());

        // Guard parse call: ts_parser_parse_string may return nullptr on failure
        TSTree *tree = nullptr;
        try {
            // Cast length to uint32_t to match Tree-sitter C API
            tree = ts_parser_parse_string(parser, nullptr, content.c_str(),
                                          static_cast<uint32_t>(content.size()));
        } catch (...) {
            tree = nullptr;
        }

        if (tree == nullptr) {
            // parsing failed for this file, skip it
            continue;
        }

        TSNode root_node = ts_tree_root_node(tree);
        // treesitterCheckForSetupFunction likely expects a valid root node
        treesitterCheckForSetupFunction(content, root_node, objectNames, setupNames,
                                        setupSignatures);

        ts_tree_delete(tree);
    }

    // Ensure sizes match before writing back to JSON
    if (objectNames.size() == setupSignatures.size() &&
        setupSignatures.size() == setupNames.size()) {
        for (size_t i = 0; i < objectNames.size(); ++i) {
            full_json[Lib]["objects"][objectNames[i]] = {setupSignatures[i], setupNames[i]};
        }
        full_json[Lib]["dependency_objects_indexed"] = true;
        full_json[Lib]["dependencies"] = json::object();
        for (const fs::path &root : librarySearchRoots) {
            if (root == completPath) {
                continue;
            }
            std::string dependencyName = root.filename().string();
            if (dependencyName.ends_with("-src")) {
                dependencyName.resize(dependencyName.size() - 4);
            }
            full_json[Lib]["dependencies"][dependencyName] = root.string();
        }
    } else {
        print("Mismatched vector sizes when collecting objects; aborting JSON update",
              Pd4WebLogLevel::PD4WEB_ERROR);
        p->ExternalObjectsJson = full_json;
        return objectNames;
    }

    // Write JSON out (guard with try/catch)
    try {
        std::ofstream out(jsonFile);
        if (out) {
            out << full_json.dump(2);
            out.close();
        } else {
            print("Unable to open objects.json for writing: " + jsonFile,
                  Pd4WebLogLevel::PD4WEB_ERROR);
        }
    } catch (const std::exception &e) {
        print(std::string("Failed writing objects.json: ") + e.what(),
              Pd4WebLogLevel::PD4WEB_ERROR);
    }

    p->ExternalObjectsJson = full_json;
    return objectNames;
}
