#include "pd4web_compiler.hpp"

// ─────────────────────────────────────
bool Pd4Web::gitRepoExists(const fs::path &path) {
    PD4WEB_LOGGER();
    struct stat info = {};
    const std::string pathStr = path.string();
    if (stat(pathStr.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        return false;
    }

    git_libgit2_init();
    git_repository *repo = nullptr;
    int result = git_repository_open_ext(&repo, pathStr.c_str(), 0, nullptr);
    if (repo) {
        git_repository_free(repo);
    }

    return result == 0;
}

// ─────────────────────────────────────
struct SubmoduleFileCheckContext {
    fs::path repoRoot;
    fs::path filePath;
    bool isInSubmodule = false;
};

// ─────────────────────────────────────
int submodule_file_check_cb(git_submodule *sm, const char *name, void *payload) {
    PD4WEB_LOGGER();
    auto *ctx = static_cast<SubmoduleFileCheckContext *>(payload);
    fs::path submodulePath = ctx->repoRoot / git_submodule_path(sm);
    fs::path fileAbs = fs::absolute(ctx->filePath);
    const std::string submoduleAbs = fs::absolute(submodulePath).string();
    std::string submodulePrefix = submoduleAbs;
    if (!submodulePrefix.empty() && submodulePrefix.back() != fs::path::preferred_separator) {
        submodulePrefix.push_back(fs::path::preferred_separator);
    }
    const std::string fileAbsStr = fileAbs.string();
    if (fs::is_directory(submodulePath) &&
        (fileAbsStr == submoduleAbs || fileAbsStr.rfind(submodulePrefix, 0) == 0)) {
        ctx->isInSubmodule = true;
        return 1;
    }
    return 0;
}

// ─────────────────────────────────────
bool Pd4Web::isFileFromGitSubmodule(const fs::path &repoRoot, const fs::path &filePath) {
    PD4WEB_LOGGER();
    git_repository *repo = nullptr;
    const std::string repoRootStr = repoRoot.string();
    if (git_repository_open(&repo, repoRootStr.c_str()) != 0) {
        return false;
    }
    SubmoduleFileCheckContext ctx{repoRoot, filePath, false};
    git_submodule_foreach(repo, submodule_file_check_cb, &ctx);
    git_repository_free(repo);
    return ctx.isInSubmodule;
}

namespace {
struct SubmoduleUpdateContext {
    bool ok = true;
};

bool updateSubmodulesRecursive(git_repository *repo);

int submodule_update_recursive_cb(git_submodule *sm, const char *name, void *payload) {
    auto *ctx = static_cast<SubmoduleUpdateContext *>(payload);

    (void)name;

    git_submodule_sync(sm);

    git_submodule_update_options update_opts = GIT_SUBMODULE_UPDATE_OPTIONS_INIT;
    update_opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    int err = git_submodule_update(sm, 1, &update_opts);
    if (err != 0) {
        ctx->ok = false;
        return err;
    }

    git_repository *subrepo = nullptr;
    err = git_submodule_open(&subrepo, sm);
    if (err != 0) {
        ctx->ok = false;
        return err;
    }

    bool nestedOk = updateSubmodulesRecursive(subrepo);
    git_repository_free(subrepo);
    if (!nestedOk) {
        ctx->ok = false;
        return -1;
    }

    return 0;
}

bool updateSubmodulesRecursive(git_repository *repo) {
    SubmoduleUpdateContext ctx{true};
    int err = git_submodule_foreach(repo, submodule_update_recursive_cb, &ctx);
    return err == 0 && ctx.ok;
}
} // namespace

// ─────────────────────────────────────
// TODO: gitRootFolderName is a bad name, I am talking about libname, or repo name
bool Pd4Web::gitClone(const std::string &url, const fs::path &gitFolderRoot,
                      const std::string &tag) {
    PD4WEB_LOGGER();

#if defined(__linux__)
    std::string cert = getCertFile();
    if (cert.empty()) {
        print("Failed to configure libgit2", Pd4WebLogLevel::PD4WEB_WARNING);
        return false;
    }
    int err = git_libgit2_opts(GIT_OPT_SET_SSL_CERT_LOCATIONS, cert.c_str(), nullptr);
    if (err < 0) {
        print("Failed to configure libgit2", Pd4WebLogLevel::PD4WEB_ERROR);
    }
#endif
    fs::path path = m_Pd4WebRoot / gitFolderRoot;
    if (gitRepoExists(path)) {
        return gitCheckout(url, gitFolderRoot, tag);
    }

    print("Cloning " + url + " inside " + path.string() + " This may take a while.",
          Pd4WebLogLevel::PD4WEB_LOG2);

    git_repository *repo = nullptr;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    int last_percent = -1;
    // fetch_opts.callbacks.transfer_progress = progress_callback;
    fetch_opts.callbacks.payload = &last_percent;

    clone_opts.fetch_opts = fetch_opts;
    clone_opts.checkout_branch = nullptr; // No automatic checkout

    const std::string pathStr = path.string();
    int error = git_clone(&repo, url.c_str(), pathStr.c_str(), &clone_opts);
    if (error != 0) {
        const git_error *e = git_error_last();
        std::string cloneErrorMessage = (e && e->message) ? e->message : "unknown error";
        print("git_clone error " + std::to_string(error) + " " + cloneErrorMessage,
              Pd4WebLogLevel::PD4WEB_WARNING);
        print("libgit2 clone failed, trying git CLI fallback", Pd4WebLogLevel::PD4WEB_WARNING);

        std::vector<std::string> args = {"clone", url, pathStr, "--recursive"};
        std::string gitPath;
#ifdef _WIN32
        char gitPathBuf[MAX_PATH] = {};
        DWORD len = SearchPathA(nullptr, "git.exe", nullptr, MAX_PATH, gitPathBuf, nullptr);
        if (len == 0 || len >= MAX_PATH) {
            print("git_clone error " + std::to_string(error) + " " + cloneErrorMessage +
                      ". And git not found in PATH",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        gitPath = gitPathBuf;
#else
        const char *path_env = std::getenv("PATH");
        if (!path_env) {
            print("git_clone error " + std::to_string(error) + " " + cloneErrorMessage +
                      ". And PATH is not available for git fallback",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }

        std::istringstream ss(path_env);
        std::string dir;
        bool gitFound = false;
        while (std::getline(ss, dir, ':')) {
            fs::path p = fs::path(dir) / "git";
            if (fs::exists(p) && fs::is_regular_file(p) && access(p.c_str(), X_OK) == 0) {
                gitPath = p.string();
                gitFound = true;
                break;
            }
        }

        if (!gitFound) {
            print("git_clone error " + std::to_string(error) + " " + cloneErrorMessage +
                      ". And git not found in PATH",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
#endif

        int r = execProcess(gitPath, args);
        if (r != 0) {
            print("git_clone error " + std::to_string(error) + " " + cloneErrorMessage +
                      ". Using git cli also failed",
                  Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        if (git_repository_open(&repo, pathStr.c_str()) != 0) {
            print("Failed to open repository " + pathStr, Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
    }

    git_repository_free(repo);
    return gitCheckout(url, gitFolderRoot, tag);
}

// ─────────────────────────────────────
bool Pd4Web::gitPull(std::string git, fs::path gitFolder) {
    PD4WEB_LOGGER();
    fs::path path = m_Pd4WebRoot / gitFolder;

    git_repository *repo = nullptr;
    const std::string pathStr = path.string();
    if (git_repository_open(&repo, pathStr.c_str()) != 0) {
        return false;
    }

    // Cria/lookup do remote origin
    git_remote *remote = nullptr;
    if (git_remote_lookup(&remote, repo, "origin") != 0) {
        if (git_remote_create(&remote, repo, "origin", git.c_str()) != 0) {
            git_repository_free(repo);
            return false;
        }
    }

    // Fetch remoto
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_ALL;
    if (git_remote_fetch(remote, nullptr, &fetch_opts, nullptr) != 0) {
        git_remote_free(remote);
        git_repository_free(repo);
        return false;
    }

    // Recupera branch atual
    git_reference *local_ref = nullptr;
    if (git_repository_head(&local_ref, repo) != 0) {
        git_remote_free(remote);
        git_repository_free(repo);
        return false;
    }

    const char *branch = git_reference_shorthand(local_ref);
    std::string remote_branch = "refs/remotes/origin/" + std::string(branch);

    // Lookup remote ref
    git_reference *remote_ref = nullptr;
    if (git_reference_lookup(&remote_ref, repo, remote_branch.c_str()) != 0) {
        git_reference_free(local_ref);
        git_remote_free(remote);
        git_repository_free(repo);
        return false;
    }

    // Lookup remote commit
    const git_oid *remote_oid = git_reference_target(remote_ref);
    git_object *target_commit = nullptr;
    if (git_object_lookup(&target_commit, repo, remote_oid, GIT_OBJECT_COMMIT) != 0) {
        git_reference_free(remote_ref);
        git_reference_free(local_ref);
        git_remote_free(remote);
        git_repository_free(repo);
        return false;
    }

    // Checkout forçado para o estado remoto
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE; // força sobrescrita

    if (git_checkout_tree(repo, target_commit, &checkout_opts) != 0) {
        git_object_free(target_commit);
        git_reference_free(remote_ref);
        git_reference_free(local_ref);
        git_remote_free(remote);
        git_repository_free(repo);
        return false;
    }

    // Reset HEAD para corresponder à origem (hard reset)
    if (git_reset(repo, target_commit, GIT_RESET_HARD, nullptr) != 0) {
        git_object_free(target_commit);
        git_reference_free(remote_ref);
        git_reference_free(local_ref);
        git_remote_free(remote);
        git_repository_free(repo);
        return false;
    }

    git_object_free(target_commit);
    git_reference_free(remote_ref);
    git_reference_free(local_ref);
    git_remote_free(remote);
    git_repository_free(repo);
    return true;
}

// ─────────────────────────────────────
std::string Pd4Web::getCurrentCommit(const fs::path &repoPath) {
    git_repository *repo = nullptr;
    git_reference *head = nullptr;
    git_object *obj = nullptr;

    const std::string repoPathStr = repoPath.string();
    if (git_repository_open(&repo, repoPathStr.c_str()) != 0) {
        return {};
    }

    // Resolve HEAD (works for detached HEAD and branches)
    if (git_revparse_single(&obj, repo, "HEAD") != 0) {
        git_repository_free(repo);
        return {};
    }

    const git_oid *oid = git_object_id(obj);
    char sha[GIT_OID_HEXSZ + 1];
    git_oid_tostr(sha, sizeof(sha), oid);

    git_object_free(obj);
    git_repository_free(repo);

    return std::string(sha);
}

// ─────────────────────────────────────
bool Pd4Web::gitCheckout(std::string git, const fs::path gitFolder, std::string tag) {
    PD4WEB_LOGGER();

    fs::path path = m_Pd4WebRoot / gitFolder;
    git_repository *repo = nullptr;

    const std::string pathStr = path.string();
    if (git_repository_open(&repo, pathStr.c_str()) != 0) {
        print("Failed to open repository " + pathStr, Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    auto try_checkout_tag = [&](git_repository *r) -> bool {
        std::string full_tag = "refs/tags/" + tag;
        git_object *tag_obj = nullptr;

        // Tenta primeiro como tag
        if (git_revparse_single(&tag_obj, r, full_tag.c_str()) != 0) {
            // Tenta como hash direto (commit SHA ou nome resolvível)
            if (git_revparse_single(&tag_obj, r, tag.c_str()) != 0) {
                return false;
            }
        }

        git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        if (git_checkout_tree(r, tag_obj, &checkout_opts) != 0) {
            git_object_free(tag_obj);
            return false;
        }

        if (git_repository_set_head_detached(r, git_object_id(tag_obj)) != 0) {
            git_object_free(tag_obj);
            return false;
        }

        git_object_free(tag_obj);
        return true;
    };

    // Primeira tentativa
    if (try_checkout_tag(repo)) {
        bool submoduleOk = updateSubmodulesRecursive(repo);
        git_repository_free(repo);
        if (!submoduleOk) {
            print("Failed to initialize submodules", Pd4WebLogLevel::PD4WEB_ERROR);
            return false;
        }
        return true;
    }

    // Falhou -> tenta pull
    if (!gitPull(git, gitFolder)) {
        git_repository_free(repo);
        print("Failed Pull\n", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // Re-open repository after pull to refresh refs/cache.
    git_repository_free(repo);
    repo = nullptr;
    if (git_repository_open(&repo, pathStr.c_str()) != 0) {
        print("Failed to reopen repository " + pathStr, Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    // Segunda tentativa após pull
    if (!try_checkout_tag(repo)) {
        git_repository_free(repo);
        print("Failed Checkout\n", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }

    bool submoduleOk = updateSubmodulesRecursive(repo);

    git_repository_free(repo);
    if (!submoduleOk) {
        print("Failed to initialize submodules", Pd4WebLogLevel::PD4WEB_ERROR);
        return false;
    }
    return true;
}
