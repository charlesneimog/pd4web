#include <thread>

#include <m_pd.h>
#include <m_imp.h>

#define PDOBJECT 1
#include <pd4web_compiler.hpp>

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
    t_outlet *out;
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
        const char *patchName = atom_getsymbol(av)->s_name;
        fs::path patchPath(patchName);

        if (!fs::exists(patchPath)) {
            logpost(x, 1, "[pd4web] patch file %s does not exist", patchName);
            return;
        }
        if (x->object_root != patchName) {
            x->pd4web->setPatchFile(patchName);
            x->pd4web->setOutputFolder(patchPath.parent_path().string());
            logpost(x, 2, "[pd4web] set patch to %s", patchName);
            x->object_root = patchName;
        }
    } else if (strcmp(s->s_name, "es6module") == 0) {
        bool es6 = atom_getint(av) != 0;
        x->pd4web->setExportES6Module(es6);

    } else if (strcmp(s->s_name, "memory") == 0) {
        int mem = atom_getint(av);
        x->pd4web->setInitialMemory(mem);
        if (mem != (int)x->memory) {
            logpost(x, 2, "[pd4web] set memory to %d", mem);
        }
        x->memory = mem;
    }

    else if (strcmp(s->s_name, "evnk") == 0) {
        for (int i = 0; i < ac; i++) {
            x->pd4web->addVirtualNumberKeyboardObject(atom_getsymbol(av + i)->s_name);
        }
    }

    else if (strcmp(s->s_name, "evtk") == 0) {
        for (int i = 0; i < ac; i++) {
            x->pd4web->addVirtualTextKeyboardObject(atom_getsymbol(av + i)->s_name);
        }
    }

    else if (strcmp(s->s_name, "patchzoom") == 0) {
        float zoom = atom_getfloat(av);
        x->pd4web->setPatchZoom(zoom);
        if (zoom != x->zoom) {
            logpost(x, 2, "[pd4web] set zoom to %.2f", zoom);
        }
        x->zoom = zoom;
    }

    else if (strcmp(s->s_name, "output") == 0) {
        const char *folder = atom_getsymbol(av)->s_name;
        x->pd4web->setOutputFolder(folder);
        if (x->object_root != folder) {
            logpost(x, 2, "[pd4web] set output folder to %s", folder);
        }
        x->object_root = folder;
    }

    else if (strcmp(s->s_name, "template") == 0) {
        unsigned tid = atom_getint(av);
        x->pd4web->setTemplateId(tid);
        if (tid != x->patch_template) {
            logpost(x, 2, "[pd4web] set template ID to %u", tid);
        }
        x->patch_template = tid;
    }

    else if (strcmp(s->s_name, "debug") == 0) {
        bool dbg = atom_getint(av) != 0;
        x->pd4web->setDebugMode(dbg);
        if (dbg != x->verbose) {
            logpost(x, 2, "[pd4web] debug mode %s", dbg ? "enabled" : "disabled");
        }
        x->verbose = dbg;
    }

    else if (strcmp(s->s_name, "failfast") == 0) {
        bool ff = atom_getint(av) != 0;
        x->pd4web->setFailFast(ff);
        // Nenhum campo armazenado, apenas log
        logpost(x, 2, "[pd4web] failfast %s", ff ? "enabled" : "disabled");
    }

    else if (strcmp(s->s_name, "gui") == 0) {
        bool gui = atom_getint(av) != 0;
        if (!gui) {
            x->pd4web->disableGuiRender();
        }
        if (gui != x->gui) {
            logpost(x, 2, "[pd4web] GUI rendering %s", gui ? "enabled" : "disabled");
        }
        x->gui = gui;
    }

    else if (strcmp(s->s_name, "clean") == 0) {
        bool clean = atom_getint(av) != 0;
        x->pd4web->setCleanBuild(clean);
        // sem log, pois não há campo armazenado
    }

    else if (strcmp(s->s_name, "server") == 0) {
        bool server = atom_getint(av) != 0;
        x->pd4web->serverPatch(server, true, "");
        logpost(x, 2, "[pd4web] Server %s", server ? "enabled" : "disabled");
        if (server) {
            pdgui_vmess("::pd_menucommands::menu_openfile", "s", "http://localhost:8082");
        }
    }

    else if (strcmp(s->s_name, "verbose") == 0) {
        logpost(x, 2, "[pd4web] not implemented yet!");
    }

    else {
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
    static int line_count = 0;
    Pd4WebObj *x = (Pd4WebObj *)obj;
    static std::string current_line;
    if (d->loglevel == Pd4WebLogLevel::PD4WEB_ERROR) {
        logpost(obj, 1, "[pd4web] %s", d->msg.c_str());
    } else if (d->loglevel != Pd4WebLogLevel::PD4WEB_VERBOSE) {
        if (d->msg != "\n") {
            if (d->msg == "[-" || d->msg == "[") {
                startpost("%s", d->msg.c_str());
            } else if (d->msg == "-") {
                poststring(d->msg.c_str());
            } else if (d->msg == "-]" || d->msg == "]") {
                poststring(d->msg.c_str());
                post("");
            } else {
                logpost(obj, 2, "[pd4web] %s", d->msg.c_str());
            }
        } else {
            post("");
        }
    } else if (d->loglevel == Pd4WebLogLevel::PD4WEB_VERBOSE) {
        if (d->msg != "\n") {
            logpost(obj, 3, "[pd4web] %s", d->msg.c_str());
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
    fs::path pd4web_path = std::string(pd4web_class->c_externdir->s_name);
    fs::path pd4webHome = pd4web_path;

    // process
    x->out = outlet_new(&x->obj, &s_anything);
    x->pd4web = new Pd4Web();
    x->pd4web->setPd4WebRootFolder(pd4web_obj_root);
    x->pd4web->setPrintCallback([x](const std::string &msg, Pd4WebLogLevel loglevel, int level) {
        Pd4WebDetachedPost *d = new Pd4WebDetachedPost();
        d->msg = msg;
        d->loglevel = loglevel;
        pd_queue_mess(&pd_maininstance, &x->obj.te_g.g_pd, d, pd4web_logcallback);
    });

    std::thread([x]() {
        try {
            x->pd4web->init();
        } catch (const std::exception &e) {
            logpost(nullptr, 0, "%s: %s", "Exception during pd4web initialization", e.what());
        } catch (...) {
            logpost(nullptr, 0, "Unknown exception during pd4web initialization.");
            return;
        }
    }).detach();

    // pd4web config
    x->cancel = false;
    x->verbose = false;
    x->memory = 256;
    x->gui = true;
    x->zoom = 1;
    x->patch_template = 0;

    x->pd4web->setInitialMemory(x->memory);
    x->pd4web->setPatchZoom(x->zoom);
    x->pd4web->setTemplateId(x->patch_template);
    x->pd4web->setDebugMode(false);
    x->pd4web->setFailFast(false);
    x->pd4web->setCleanBuild(false);

    return x;
}

// ─────────────────────────────────────
static void pd4web_free(Pd4WebObj *x) {
    x->pd4web->serverPatch(false, true, "");
    delete x->pd4web;
}

// ─────────────────────────────────────
extern "C" void setup_pd4web0x2ecompiler(void) {

    post("[pd4web] by Charles K. Neimog, v%d.%d.%s", PD4WEB_VERSION_MAJOR, PD4WEB_VERSION_MINOR,
         PD4WEB_VERSION_PATCH);
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
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("es6module"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("evnk"), A_GIMME, 0);
    class_addmethod(pd4web_class, (t_method)pd4web_set, gensym("evtk"), A_GIMME, 0);

    class_addbang(pd4web_class, (t_method)pd4web_compile);
}
