#include <string>

#include <m_pd.h>

#include <g_canvas.h>

static t_widgetbehavior graphic_widgetbehavior;
static t_class *graphic_class, *graphic_proxy;

// ─────────────────────────────────────
class graphic {
  public:
    t_object x_obj;
    t_clock *x_clock;

};

// ─────────────────────────────────────
typedef struct _graphic_proxy { 
    t_object    p_obj;
    t_symbol    *p_sym;
    t_clock     *p_clock;
    graphic     *p_gobj;
} t_graphic_proxy;

// ─────────────────────────────────────
void graphic_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2) {
    int x1 = 0x7fffffff, y1 = 0x7fffffff, x2 = -0x7fffffff, y2 = -0x7fffffff;
    t_glist *x = (t_glist *)z;
    text_widgetbehavior.w_getrectfn(z, glist, &x1, &y1, &x2, &y2);
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

// ─────────────────────────────────────
void graphic_draw(graphic *x, t_glist *glist) {
    t_canvas *canvas = glist_getcanvas(glist);
    std::string tag = "graphic_" + std::to_string(reinterpret_cast<unsigned long>(x));
    int x1, y1, width, height;
    graphic_getrect((t_gobj *)x, glist, &x1, &y1, &width, &height);
    pdgui_vmess(0, "crr iiii rs",
                      canvas,
                      "create", "rectangle",
                      x1, y1, width + 180, height + 180,
                      "-tags", tag.c_str());
}

// ─────────────────────────────────────
void graphic_displace(t_gobj *z, t_glist *glist, int dx, int dy) {
    graphic *x = (graphic *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
}

// ─────────────────────────────────────
void graphic_select(t_gobj *z, t_glist *glist, int state) {
    graphic *x = (graphic *)z;
    if (state) {
        int x1, y1, width, height;
        graphic_getrect(z, glist, &x1, &y1, &width, &height);
        pdgui_vmess(0, "crr iiii rs",
                        glist_getcanvas(glist),
                        "create", "rectangle",
                        x1 - 4, y1 - 4,
                        x1 + width + 4,
                        "-outline", "blue",
                        "-tags", "select_rect");
    } else {
        pdgui_vmess(0, "crr rs",
                          glist_getcanvas(glist),
                          "delete", "select_rect");
    }
}

// ─────────────────────────────────────
void graphic_activate(t_gobj *z, t_glist *glist, int state) {
}

// ─────────────────────────────────────
void graphic_delete(t_gobj *z, t_glist *glist) {
}

// ─────────────────────────────────────
void graphic_vis(t_gobj *z, t_glist *glist, int flag) {
    graphic *x = (graphic *)z;
    if (flag) {
        graphic_draw(x, glist);
    } else {
        // delete the graphic?
    }
}

// ─────────────────────────────────────
int graphic_click(t_gobj *z, t_glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    // Implement click behavior if needed
    return 0;
}

// ─────────────────────────────────────
void *graphic_new(void) {
    graphic *x = (graphic *)pd_new(graphic_class);
    x->x_clock = clock_new(x, (t_method)graphic_draw);
    return (void *)x;
}

// ─────────────────────────────────────
void graphic_proxynew(t_py4pd_edit_proxy *p) {
    pd_unbind(&p->p_obj.ob_pd, p->p_sym);
    clock_free(p->p_clock);
    pd_free(&p->p_obj.ob_pd);
}

// ─────────────────────────────────────
t_graphic_proxy *graphic_proxyedit(graphic *x, t_symbol *s) {
    t_graphic_proxy *p = (t_graphic_proxy *)pd_new(graphic_proxy);
    p->p_cnv = x;
    pd_bind(&p->p_obj.ob_pd, p->p_sym = s);
    p->p_clock = clock_new(p, (t_method)Py4pdPic_EditProxyFree);
    return (p);
}


// ─────────────────────────────────────
extern "C" void graphic_setup(void) {
    graphic_class = class_new(gensym("graphic"),
                              (t_newmethod)graphic_new,
                              0, sizeof(graphic),
                              CLASS_DEFAULT, A_GIMME, 0);

    graphic_widgetbehavior.w_getrectfn = graphic_getrect;
    graphic_widgetbehavior.w_visfn = graphic_vis;

    graphic_widgetbehavior.w_displacefn = graphic_displace;
    //graphic_widgetbehavior.w_selectfn = graphic_select;
    //graphic_widgetbehavior.w_activatefn = graphic_activate;
    //graphic_widgetbehavior.w_deletefn = graphic_delete;
    //graphic_widgetbehavior.w_clickfn = graphic_click;
    class_setwidget(graphic_class, &graphic_widgetbehavior);
    Py4pdEditProxy = class_new(0, 0, 0, sizeof(t_py4pd_edit_proxy), CLASS_NOINLET | CLASS_PD, 0);

        class_addanything(Py4pdEditProxy, Py4pdPic_EditProxyAny);

}
