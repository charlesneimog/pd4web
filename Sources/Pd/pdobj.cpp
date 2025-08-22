#include <thread>

#include <m_pd.h>
#include <m_imp.h>

#define PDOBJECT 1
#include <pd4web_compiler.hpp>

#define PD4WEB_EXTERNAL_VERSION "2.4.0"
static t_class *pd4web_class;

// ─────────────────────────────────────
class Pd4WebObj {
  public:
    t_object obj;
    std::string object_root;
    bool cancel;
    bool verbose;
    bool gui;
    unsigned memory;
    float zoom;
    unsigned patch_template;
    Pd4Web *pd4web;
};

// ─────────────────────────────────────
struct Pd4WebDetachedPost {
    std::string msg;
    Pd4WebLogLevel loglevel;
};

// ─────────────────────────────────────
static void pd4web_set(Pd4WebObj *x, t_symbol *s, int ac, t_atom *av) {
    if (strcmp(s->s_name, "patch") == 0) {
        x->pd4web->setPatchFile(atom_getsymbol(av)->s_name);
        fs::path patchPath(atom_getsymbol(av)->s_name);
        if (!fs::exists(patchPath)) {
            logpost(x, 1, "[pd4web] patch file %s does not exist", atom_getsymbol(av)->s_name);
            return;
        }
        x->pd4web->setOutputFolder(patchPath.parent_path().string());
        logpost(x, 2, "[pd4web] set patch to %s", atom_getsymbol(av)->s_name);
    } else if (strcmp(s->s_name, "memory") == 0) {
        int mem = atom_getint(av);
        x->pd4web->setInitialMemory(mem);
        logpost(x, 2, "[pd4web] set memory to %d", mem);
    } else if (strcmp(s->s_name, "patchzoom") == 0) {
        int zoom = atom_getint(av);
        x->pd4web->setPatchZoom(zoom);
        logpost(x, 2, "[pd4web] set zoom to %d", zoom);
    } else if (strcmp(s->s_name, "output") == 0) {
        x->pd4web->setOutputFolder(atom_getsymbol(av)->s_name);
        logpost(x, 2, "[pd4web] set output folder to %s", atom_getsymbol(av)->s_name);
    } else if (strcmp(s->s_name, "template") == 0) {
        int tid = atom_getint(av);
        x->pd4web->setTemplateId(tid);
        logpost(x, 2, "[pd4web] set template ID to %d", tid);
    } else if (strcmp(s->s_name, "debug") == 0) {
        bool dbg = atom_getint(av) != 0;
        x->pd4web->setDebugMode(dbg);
        logpost(x, 2, "[pd4web] debug mode %s", dbg ? "enabled" : "disabled");
    } else if (strcmp(s->s_name, "failfast") == 0) {
        bool ff = atom_getint(av) != 0;
        x->pd4web->setFailFast(ff);
        logpost(x, 2, "[pd4web] failfast %s", ff ? "enabled" : "disabled");
    } else if (strcmp(s->s_name, "gui") == 0) {
        bool gui = atom_getint(av) != 0;
        if (!gui) {
            x->pd4web->disableGuiRender();
        }
        logpost(x, 2, "[pd4web] GUI rendering %s", gui ? "enabled" : "disabled");
    } else if (strcmp(s->s_name, "clean") == 0) {
        bool clean = atom_getint(av) != 0;
        x->pd4web->setCleanBuild(clean);
    } else if (strcmp(s->s_name, "verbose") == 0) {
        logpost(x, 2, "[pd4web] not implemented yet!");
    } else if (strcmp(s->s_name, "server") == 0) {
        bool server = atom_getint(av) != 0;
        x->pd4web->serverPatch(server);
        logpost(x, 2, "[pd4web] server %s", server ? "enabled" : "disabled");
        if (server) {
            pdgui_vmess("::pd_menucommands::menu_openfile", "s", "http://localhost:8080");
        }
    } else {
        logpost(x, 1, "[pd4web] unknown option: %s", s->s_name);
    }
}

// ─────────────────────────────────────
static void pd4web_compile(Pd4WebObj *x) {
    std::thread([x]() { x->pd4web->compilePatch(); }).detach();
}

// ─────────────────────────────────────
static void pd4web_logcallback(t_pd *obj, void *data) {
    Pd4WebDetachedPost *d = (Pd4WebDetachedPost *)data;

    if (d->loglevel == Pd4WebLogLevel::PD4WEB_ERROR) {
        logpost(obj, 1, "[pd4web] %s", d->msg.c_str());
    } else if (d->loglevel != Pd4WebLogLevel::PD4WEB_VERBOSE) {
        if (d->msg != "\n") {
            logpost(obj, 2, "[pd4web] %s", d->msg.c_str());
        } else {
            post("");
        }
    }
    delete d;
}

// ─────────────────────────────────────
static void *pd4web_new() {
    Pd4WebObj *x = (Pd4WebObj *)pd_new(pd4web_class);
    std::string pd4web_obj_root = std::string(pd4web_class->c_externdir->s_name) + "/Pd4Web/";

#if defined(__APPLE__) || defined(__linux__)
    std::string home = std::getenv("HOME");
    std::filesystem::path pd4webHome = std::filesystem::path(home) / ".local" / "share" / "pd4web";
#else
    std::string appdata = std::getenv("APPDATA");
    std::filesystem::path pd4webHome = std::filesystem::path(appdata) / "pd4web";
    std::filesystem::create_directories(pd4webHome);
#endif

    // process
    x->pd4web = new Pd4Web(pd4webHome.string());
    x->pd4web->setPd4WebFilesFolder(pd4web_obj_root);
    x->pd4web->setPrintCallback([x](const std::string &msg, Pd4WebLogLevel color, int level) {
        Pd4WebDetachedPost *d = new Pd4WebDetachedPost();
        d->msg = msg;
        d->loglevel = color;
        pd_queue_mess(&pd_maininstance, &x->obj.te_g.g_pd, d, pd4web_logcallback);
    });

    std::thread([x]() { x->pd4web->init(); }).detach();

    // pd4web config
    x->cancel = false;
    x->verbose = false;
    x->memory = 256;
    x->gui = true;
    x->zoom = 2;
    x->patch_template = 0;
    return x;
}

// ─────────────────────────────────────
static void pd4web_free(Pd4WebObj *x) {
    delete x->pd4web;
}

// ─────────────────────────────────────
extern "C" void setup_pd4web0x2ecompiler(void) {
    post("pd4web by Charles K. Neimog");
    pd4web_class = class_new(gensym("pd4web.compiler"), (t_newmethod)pd4web_new,
                             (t_method)pd4web_free, sizeof(Pd4WebObj), CLASS_DEFAULT, A_NULL);

    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("patch"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("memory"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("patchzoom"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("output"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("template"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("debug"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("failfast"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("verbose"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("clean"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("gui"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("server"), A_GIMME, 0);

    class_addbang(pd4web_class, (t_method)pd4web_compile);
}
