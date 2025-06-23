#include "pd4web.hpp"

// ──────────────────────────────────────────
static int progress_callback(const git_transfer_progress *stats, void *payload) {
    LOG(__PRETTY_FUNCTION__);
    if (stats->total_objects == 0)
        return 0;

    int percent = (stats->received_objects * 100) / stats->total_objects;
    int *last_percent = static_cast<int *>(payload);

    if (percent != *last_percent) {
        *last_percent = percent;
#ifdef PDOBJECT
        post("Downloading: %d...", percent);
#else
        std::cout << "Download: " << percent << "%" << std::endl;
#endif
    }
    return 0;
}

// ─────────────────────────────────────
bool Pd4Web::gitRepoExists(const std::string &path) {
    LOG(__PRETTY_FUNCTION__);
    struct stat info;
    if (stat(path.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        return false;
    }

    git_libgit2_init();
    git_repository *repo = nullptr;
    int result = git_repository_open_ext(&repo, path.c_str(), 0, nullptr);
    if (repo)
        git_repository_free(repo);

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
    LOG(__PRETTY_FUNCTION__);
    auto *ctx = static_cast<SubmoduleFileCheckContext *>(payload);
    fs::path submodulePath = ctx->repoRoot / git_submodule_path(sm);
    fs::path fileAbs = fs::absolute(ctx->filePath);
    if (fs::is_directory(submodulePath) &&
        fileAbs.string().starts_with(fs::absolute(submodulePath).string())) {
        ctx->isInSubmodule = true;
        return 1;
    }
    return 0;
}

// ─────────────────────────────────────
bool Pd4Web::isFileFromGitSubmodule(const fs::path &repoRoot, const fs::path &filePath) {
    LOG(__PRETTY_FUNCTION__);
    git_repository *repo = nullptr;
    if (git_repository_open(&repo, repoRoot.c_str()) != 0)
        return false;

    SubmoduleFileCheckContext ctx{repoRoot, filePath, false};

    git_submodule_foreach(repo, submodule_file_check_cb, &ctx);

    git_repository_free(repo);
    return ctx.isInSubmodule;
}

// ─────────────────────────────────────
bool Pd4Web::gitClone(std::string url, std::string gitFolder, std::string tag) {
    LOG(__PRETTY_FUNCTION__);
    std::string path = m_Pd4WebRoot + gitFolder;
    if (gitRepoExists(path)) {
        gitCheckout(url, gitFolder, tag);
        return true;
    }

    git_repository *repo = nullptr;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    int last_percent = -1;
    fetch_opts.callbacks.transfer_progress = progress_callback;
    fetch_opts.callbacks.payload = &last_percent;

    clone_opts.fetch_opts = fetch_opts;
    clone_opts.checkout_branch = nullptr; // Não faz checkout automático

    int error = git_clone(&repo, url.c_str(), path.c_str(), &clone_opts);
    if (error != 0) {
        const git_error *e = git_error_last();
        fprintf(stderr, "git_clone error %d/%d: %s\n", error, e ? e->klass : 0,
                e ? e->message : "unknown");
        return false;
    }

    // ↓↓↓ Atualiza submódulos (clone recursivo)
    git_submodule_foreach(
        repo,
        [](git_submodule *sm, const char *name, void *payload) -> int {
            std::string submodname = name;
            LOG("Initializing submodule " + submodname);
            int err = git_submodule_update(sm, 1, nullptr); // 1 = initialize
            if (err != 0) {
                const git_error *e = git_error_last();
                fprintf(stderr, "Failed to update submodule '%s': %s\n", name,
                        e ? e->message : "unknown");
            }
            return err;
        },
        nullptr);

    // Checkout manual da tag
    git_object *obj = nullptr;
    std::string ref = "refs/tags/" + tag;
    error = git_revparse_single(&obj, repo, ref.c_str());
    if (error != 0) {
        error = git_revparse_single(&obj, repo, tag.c_str());
        if (error != 0) {
            fprintf(stderr, "Error resolving tag or commit '%s'\n", tag.c_str());
            git_repository_free(repo);
            return false;
        }
    }

    error = git_checkout_tree(repo, obj, nullptr);
    if (error != 0) {
        fprintf(stderr, "Error checking out tree for tag '%s'\n", tag.c_str());
        git_object_free(obj);
        git_repository_free(repo);
        return false;
    }

    git_object_free(obj);
    git_repository_free(repo);
    if (gitRepoExists(path)) {
        gitCheckout(url, gitFolder, tag);
        return true;
    } else {
        return false;
    }
}

// ─────────────────────────────────────
bool Pd4Web::gitPull(std::string git, std::string gitFolder) {
    LOG(__PRETTY_FUNCTION__);
    std::string path = m_Pd4WebRoot + gitFolder;

    git_repository *repo = nullptr;
    if (git_repository_open(&repo, path.c_str()) != 0) {
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
bool Pd4Web::gitCheckout(std::string git, std::string gitFolder, std::string tag) {
    LOG(__PRETTY_FUNCTION__);

    std::string path = m_Pd4WebRoot + gitFolder;
    git_repository *repo = nullptr;

    if (git_repository_open(&repo, path.c_str()) != 0) {
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
        git_repository_free(repo);
        return true;
    }

    // Falhou -> tenta pull
    if (!gitPull(git, gitFolder)) {
        git_repository_free(repo);
        printf("Falhou ao dar Pull\n");
        return false;
    }

    // Segunda tentativa após pull
    if (!try_checkout_tag(repo)) {
        git_repository_free(repo);
        printf("Falhou ao dar Checkout\n");
        return false;
    }

    git_repository_free(repo);
    return true;
}
