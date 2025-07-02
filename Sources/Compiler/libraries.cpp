#include "pd4web.hpp"

// ─────────────────────────────────────
bool Pd4Web::libIsSupported(std::string libName) {
    for (Library lib : m_Libraries) {
        if (lib.Name == libName) {
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::downloadSupportedLib(std::string libName) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    for (Library Lib : m_Libraries) {
        if (Lib.Name == libName) {
            bool ok = gitClone(Lib.Url, Lib.Name, Lib.Version);
            if (!ok) {
                print("Failed to clone library'" + libName + "'", Pd4WebLogLevel::ERROR);
                return false;
            }
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────
bool Pd4Web::getSupportedLibraries(std::shared_ptr<Patch> &Patch) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    m_Libraries.clear();

    std::ifstream file(Patch->Pd4WebFiles / "Libraries" / "Libraries.yaml");
    if (!file) {
        print("Failed to open libraries file", Pd4WebLogLevel::ERROR);
        return false;
    }

    YamlNode node = fkyaml::node::deserialize(file);
    if (!node.contains("Libraries")) {
        print("YAML does not have libraries", Pd4WebLogLevel::ERROR);
        return false;
    }

    m_LibrariesNode = node.at("Libraries");
    m_SourcesNode = node.at("Sources");

    if (!m_LibrariesNode.is_sequence()) {
        print("YAML file has no sequence", Pd4WebLogLevel::ERROR);
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

        std::string url = m_SourcesNode[newLib.Source].get_value<std::string>();
        std::string urlGit = formatLibUrl(url, newLib.Developer, newLib.Repository);
        urlGit += ".git";
        newLib.Url = urlGit;

        m_Libraries.push_back(newLib);
    }

    for (const std::string &declared : Patch->DeclaredLibs) {
        if (supportedLibraries.find(declared) == supportedLibraries.end()) {
            print("Library '" + declared + "' is not supported", Pd4WebLogLevel::ERROR);
            return false;
        } else {
            for (Library Lib : m_Libraries) {
                if (Lib.Name == declared) {
                    bool ok = gitClone(Lib.Url, Lib.Name, Lib.Version);
                    if (!ok) {
                        print("Failed to clone library'" + declared + "'", Pd4WebLogLevel::ERROR);
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
std::vector<fs::path> Pd4Web::findLuaObjects(std::shared_ptr<Patch> &Patch, fs::path dir,
                                             PatchLine &pl) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
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
std::vector<std::string> Pd4Web::listAbstractionsInLibrary(std::shared_ptr<Patch> &p,
                                                           std::string Lib) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    std::vector<std::string> absNames;
    const std::string jsonFile = "/home/neimog/Documents/Git/pd4web/Sources/Compiler/objects.json";
    json full_json;
    std::ifstream in(jsonFile);
    if (in.is_open()) {
        in >> full_json;
        in.close();
        if (full_json.contains(Lib)) {
            if (full_json[Lib].contains("abstractions")) {
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

    std::string completPath = m_Pd4WebRoot + Lib;
    print("Listing all Abstractions inside '" + Lib +
              "'. This is done once for library and will take a while",
          Pd4WebLogLevel::LOG2, p->printLevel + 1);

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
void Pd4Web::treesitterCheckForSetupFunction(std::string &content, TSNode node,
                                             std::vector<std::string> &objectNames,
                                             std::vector<std::string> &setupNames,
                                             std::vector<std::string> &setupSignatures) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);

    if (ts_node_is_null(node))
        return;

    if (strcmp(ts_node_type(node), "call_expression") == 0) {
        TSNode func_node = ts_node_child_by_field_name(node, "function", 8);
        if (ts_node_is_null(func_node))
            return;

        std::string func_text(content.data() + ts_node_start_byte(func_node),
                              ts_node_end_byte(func_node) - ts_node_start_byte(func_node));

        bool is_class_new = func_text.find("class_new") != std::string::npos;
        bool is_class_addcreator = func_text.find("class_addcreator") != std::string::npos;
        if (!is_class_new && !is_class_addcreator)
            return;

        TSNode args_node = ts_node_child_by_field_name(node, "arguments", 9);
        if (ts_node_is_null(args_node))
            return;

        uint32_t args_count = ts_node_named_child_count(args_node);
        uint32_t target_arg_index = is_class_new ? 0 : 1;
        if (args_count <= target_arg_index)
            return;

        TSNode target_arg = ts_node_named_child(args_node, target_arg_index);
        if (strcmp(ts_node_type(target_arg), "call_expression") != 0)
            return;

        TSNode inner_func = ts_node_child_by_field_name(target_arg, "function", 8);
        if (ts_node_is_null(inner_func))
            return;

        std::string inner_func_text(content.data() + ts_node_start_byte(inner_func),
                                    ts_node_end_byte(inner_func) - ts_node_start_byte(inner_func));
        if (inner_func_text != "gensym")
            return;

        TSNode gensym_args = ts_node_child_by_field_name(target_arg, "arguments", 9);
        if (ts_node_named_child_count(gensym_args) < 1)
            return;

        TSNode string_arg = ts_node_named_child(gensym_args, 0);
        if (strcmp(ts_node_type(string_arg), "string_literal") != 0)
            return;

        std::string object_name(content.data() + ts_node_start_byte(string_arg) + 1,
                                ts_node_end_byte(string_arg) - ts_node_start_byte(string_arg) - 2);

        if (std::find(objectNames.begin(), objectNames.end(), object_name) != objectNames.end()) {
            return;
        }

        objectNames.push_back(object_name);

        // ← Find function_definition parent
        TSNode current = node;
        while (!ts_node_is_null(current)) {
            if (strcmp(ts_node_type(current), "function_definition") == 0)
                break;
            current = ts_node_parent(current);
        }

        if (ts_node_is_null(current))
            return;

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
std::vector<std::string> Pd4Web::listObjectsInLibrary(std::shared_ptr<Patch> &p, std::string Lib) {
    print(__PRETTY_FUNCTION__, Pd4WebLogLevel::VERBOSE);
    std::vector<std::string> objectNames;
    std::vector<std::string> setupSignatures;
    std::vector<std::string> setupNames;

    std::string completPath = m_Pd4WebRoot + Lib;
    if (!fs::exists(completPath) || !fs::is_directory(completPath)) {
        print("Library '" + Lib + "' not found", Pd4WebLogLevel::ERROR);
        return objectNames;
    }

    // TODO: Fix this path
    const std::string jsonFile = "/home/neimog/Documents/Git/pd4web/Sources/Compiler/objects.json";
    json full_json;
    std::ifstream in(jsonFile);
    if (in.is_open()) {
        in >> full_json;
        in.close();
        if (full_json.contains(Lib)) {
            if (full_json[Lib].contains("objects")) {
                const auto &libEntry = full_json[Lib]["objects"];
                std::vector<std::string> keys;
                for (auto it = libEntry.begin(); it != libEntry.end(); ++it) {
                    keys.push_back(it.key());
                }
                p->ExternalObjectsJson = full_json;
                return keys;
            }
        }
    }

    print("Listing all Objects inside '" + Lib +
              "'. This is done once for library and will take a while",
          Pd4WebLogLevel::LOG2, p->printLevel + 1);

    for (const auto &entry : fs::recursive_directory_iterator(completPath)) {
        if (!isFileFromGitSubmodule(completPath, entry.path()) && entry.is_regular_file()) {
            auto ext = entry.path().extension();
            TSParser *parser = nullptr;

            if (ext == ".c") {
                parser = m_cParser;
            } else if (ext == ".cpp") {
                parser = m_cppParser;
            } else {
                continue;
            }

            std::ifstream inFile(entry.path());
            if (!inFile)
                continue;

            std::string content((std::istreambuf_iterator<char>(inFile)),
                                std::istreambuf_iterator<char>());

            TSTree *tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.size());
            TSNode root_node = ts_tree_root_node(tree);
            treesitterCheckForSetupFunction(content, root_node, objectNames, setupNames,
                                            setupSignatures);
            ts_tree_delete(tree);
        }
    }

    int objSize = objectNames.size();
    int sigSize = setupSignatures.size();
    int nameSize = setupNames.size();

    if (objSize == sigSize && sigSize == nameSize) {
        for (size_t i = 0; i < objSize; ++i) {
            full_json[Lib]["objects"][objectNames[i]] = {setupSignatures[i], setupNames[i]};
        }
    } else {
        print("This should not happend please report", Pd4WebLogLevel::ERROR);
        p->ExternalObjectsJson = full_json;
        return objectNames;
    }

    std::ofstream out(jsonFile);
    out << full_json.dump(2);
    out.close();
    p->ExternalObjectsJson = full_json;
    return objectNames;
}
