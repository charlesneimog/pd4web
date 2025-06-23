#include "pd4web.hpp"

// ─────────────────────────────────────
// TODO: Rename to getSuppotedLibraries();
bool Pd4Web::getSupportedLibraries(std::shared_ptr<Patch> &Patch) {
    LOG(__PRETTY_FUNCTION__);
    m_Libraries.clear();

    // TODO: get true patch
    std::ifstream file("/home/neimog/Documents/Git/pd4web/Sources/Libraries/Libraries.yaml");
    if (!file) {
        LOG("Failed to open libraries file")
        return false;
    }

    YamlNode node = fkyaml::node::deserialize(file);
    if (!node.contains("Libraries")) {
        LOG("YAML does not have libraries")
        return false;
    }

    m_LibrariesNode = node.at("Libraries");
    m_SourcesNode = node.at("Sources");

    if (!m_LibrariesNode.is_sequence()) {
        LOG("YAML file has no sequence");
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

    for (const std::string &declared : m_DeclaredLibraries) {
        if (supportedLibraries.find(declared) == supportedLibraries.end()) {
            LOG("Library '" + declared + "' is not supported")
            return false;
        } else {
            for (Library Lib : m_Libraries) {
                if (Lib.Name == declared) {
                    bool ok = gitClone(Lib.Url, Lib.Name, Lib.Version);
                    if (!ok) {
                        LOG("Failed to clone library '" + declared + "'");
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
            std::string objName = entry.path().stem().c_str();
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
std::vector<std::string> Pd4Web::listObjectsInLibrary(std::string Lib) {
    LOG(__PRETTY_FUNCTION__);
    std::vector<std::string> objectNames;

    std::string completPath = m_Pd4WebRoot + Lib;
    if (!fs::exists(completPath) || !fs::is_directory(completPath)) {
        LOG("Library '" + Lib + "' not found");
        return objectNames;
    }

    print("Listing all objects and Abstractions inside '" + Lib +
              "'. This is done once and will take a while",
          Pd4WebColor::GREEN);
    for (const auto &entry : fs::recursive_directory_iterator(completPath)) {
        if (!isFileFromGitSubmodule(completPath, entry.path())) {
            if (entry.is_regular_file()) {
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

                TSTree *tree =
                    ts_parser_parse_string(parser, nullptr, content.c_str(), content.size());
                TSNode root_node = ts_tree_root_node(tree);

                std::function<void(TSNode)> walk = [&](TSNode node) {
                    if (ts_node_is_null(node))
                        return;

                    if (strcmp(ts_node_type(node), "call_expression") == 0) {
                        TSNode func_node = ts_node_child_by_field_name(node, "function", 8);
                        if (ts_node_is_null(func_node))
                            return;

                        std::string func_text(content.data() + ts_node_start_byte(func_node),
                                              ts_node_end_byte(func_node) -
                                                  ts_node_start_byte(func_node));

                        bool is_class_new = func_text.find("class_new") != std::string::npos;
                        bool is_class_addcreator =
                            func_text.find("class_addcreator") != std::string::npos;
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
                                                    ts_node_end_byte(inner_func) -
                                                        ts_node_start_byte(inner_func));
                        if (inner_func_text != "gensym")
                            return;

                        TSNode gensym_args =
                            ts_node_child_by_field_name(target_arg, "arguments", 9);
                        if (ts_node_named_child_count(gensym_args) < 1)
                            return;

                        TSNode string_arg = ts_node_named_child(gensym_args, 0);
                        if (strcmp(ts_node_type(string_arg), "string_literal") != 0)
                            return;

                        std::string object_name(content.data() + ts_node_start_byte(string_arg) + 1,
                                                ts_node_end_byte(string_arg) -
                                                    ts_node_start_byte(string_arg) - 2);

                        // Walk up the tree to find the containing function
                        std::string function_name = "<unknown>";
                        TSNode current = node;
                        while (!ts_node_is_null(current)) {
                            if (strcmp(ts_node_type(current), "function_definition") == 0) {
                                TSNode decl =
                                    ts_node_child_by_field_name(current, "declarator", 10);
                                if (!ts_node_is_null(decl)) {
                                    function_name = std::string(
                                        content.data() + ts_node_start_byte(decl),
                                        ts_node_end_byte(decl) - ts_node_start_byte(decl));
                                }
                                break;
                            }
                            current = ts_node_parent(current);
                        }

                        std::string result = function_name + ": " + object_name;
                        if (std::find(objectNames.begin(), objectNames.end(), object_name) ==
                            objectNames.end()) {
                            objectNames.push_back(object_name);
                        }
                    }

                    uint32_t child_count = ts_node_named_child_count(node);
                    for (uint32_t i = 0; i < child_count; ++i) {
                        walk(ts_node_named_child(node, i));
                    }
                };

                walk(root_node);
                ts_tree_delete(tree);
            }
        }
    }

    std::vector<YamlNode> entries;
    for (const auto &obj : objectNames) {
        std::string quoted = "'" + obj + "'";
        entries.emplace_back(YamlNode(quoted));
    }

    YamlNode objNode(entries);
    fs::path yamlPath = m_Pd4WebRoot + "Libraries.yaml";
    std::ifstream ifs(yamlPath);
    YamlNode n;

    if (!ifs) {
        // arquivo não existe, cria estrutura nova
        n = YamlNode::mapping();
        n[Lib] = YamlNode::mapping();
        n[Lib]["objects"] = objNode;
    } else {
        n = YamlNode::deserialize(ifs);
        YamlNode LibNode = n[Lib];
        if (LibNode == nullptr || LibNode.get_type() != fkyaml::node_type::MAPPING) {
            n[Lib] = YamlNode::mapping();
        }
        n[Lib]["objects"] = objNode;

        YamlNode PureData = n["pure-data/src"]["objects"];
        std::vector<std::string> PdObjects;
        for (int i = 0; i < PureData.size(); i++) {
            std::string pdObj = PureData.at(i).get_value<std::string>();
        }

        std::vector<YamlNode> entries;
        for (int i = 0; i < PureData.size(); i++) {
            std::string obj = PureData.at(i).get_value<std::string>();
            std::string quoted = "'" + obj + "'";
            entries.emplace_back(YamlNode(quoted));
        }
        n["pure-data/src"]["objects"] = YamlNode::sequence(entries);
    }

    std::ofstream ofs(yamlPath);
    ofs << n;

    return objectNames;
}

// ─────────────────────────────────────
static bool matchSetupFunction(const fs::path &file, const std::string &functionName) {
    LOG(__PRETTY_FUNCTION__);
    std::ifstream inFile(file);
    if (!inFile)
        return false;

    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    std::regex pattern1("void\\s*" + functionName + "\\s*\\(\\s*void\\s*\\)");
    std::regex pattern2("void\\s+" + functionName + "\\s*\\(\\s*\\)");

    return std::regex_search(content, pattern1) || std::regex_search(content, pattern2);
}

// ─────────────────────────────────────
bool Pd4Web::findSetupFunction(std::string objName, std::string LibName) {
    LOG(__PRETTY_FUNCTION__);
    std::string completPath = m_Pd4WebRoot + LibName;
    if (!fs::exists(completPath) || !fs::is_directory(completPath)) {
        LOG("Library '" + LibName + "' not found");
        return false;
    }

    std::string functionName = objName;
    std::replace(functionName.begin(), functionName.end(), '~', '_');
    functionName += "_setup";

    size_t dotPos;
    while ((dotPos = functionName.find('.')) != std::string::npos) {
        functionName.replace(dotPos, 1, "0x2e");
    }

    std::vector<fs::path> files;
    for (const auto &entry : fs::recursive_directory_iterator(completPath)) {
        if (!isFileFromGitSubmodule(completPath, entry.path())) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension();
                if (ext == ".c" || ext == ".cpp") {
                    if (matchSetupFunction(entry.path(), functionName)) {
                        LOG("Found '" + functionName + "' in '" + entry.path().string() + "'");
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
