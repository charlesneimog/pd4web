/** @file pdlua_gfx.h
 *  @brief pdlua_gfx -- an extension to pdlua that allows GUI rendering and interaction in pure-data
 * and plugdata
 *  @author Timothy Schoen <timschoen123@gmail.com> and Charles K. Neimog
 *  @date 2023
 *
 * Copyright (C) 2023 Timothy Schoen <timschoen123@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/*
 * This file is copied to Pd4Web/Externals/pdlua, we also, for now, change the #include
 * <pdlua_gfx.h> to <pd4weblua_gfx.c> inside the Builder.py file
 */

#include <math.h>
#include <string.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/threading.h>

#if __has_include(<pd4web_config.h>)
#include <pd4web_config.h>
#else
#include "config.h"
#endif

#include <m_pd.h>

#include <g_canvas.h>
#include <s_stuff.h>

// ╭─────────────────────────────────────╮
// │                PDLUA                │
// ╰─────────────────────────────────────╯
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
static void mylua_error(lua_State *L, t_pdlua *o, const char *descr);
static int gfx_initialize(t_pdlua *obj);
static int set_size(lua_State *L);
static int get_size(lua_State *L);
static int start_paint(lua_State *L);
static int end_paint(lua_State *L);
static int set_color(lua_State *L);

static int fill_ellipse(lua_State *L);
static int stroke_ellipse(lua_State *L);
static int fill_all(lua_State *L);
static int fill_rect(lua_State *L);
static int stroke_rect(lua_State *L);
static int fill_rounded_rect(lua_State *L);
static int stroke_rounded_rect(lua_State *L);

static int draw_line(lua_State *L);
static int draw_text(lua_State *L);
static int draw_svg(lua_State *L);

static int start_path(lua_State *L);
static int line_to(lua_State *L);
static int quad_to(lua_State *L);
static int cubic_to(lua_State *L);
static int close_path(lua_State *L);
static int stroke_path(lua_State *L);
static int fill_path(lua_State *L);

static int translate(lua_State *L);
static int scale(lua_State *L);
static int reset_transform(lua_State *L);

static int free_path(lua_State *L);
static void pdlua_gfx_clear(t_pdlua *obj, int layer, int removed);
void pdlua_gfx_repaint(t_pdlua *o, int firsttime);

typedef struct {
    uint64_t id;
    t_pdlua *object;
} t_render_object_entry;
static t_render_object_entry render_objects[256];

void pdlua_gfx_process_recovery(void) {
    uint64_t id = 0;
    int layer = -1;
    if (!TakeRenderRecovery(&id, &layer)) return;
    (void)layer; // Pd-Lua repaint callbacks publish their complete current layer set.
    for (int i = 0; i < 256; ++i) {
        if (render_objects[i].id == id && render_objects[i].object) {
            pdlua_gfx_repaint(render_objects[i].object, 0);
            return;
        }
    }
}

void pdlua_gfx_free(t_pdlua_gfx *gfx) {
    if (!gfx->render_object_id) return;
    RemoveRenderObject(gfx->render_object_id);
    for (int i = 0; i < 256; ++i) {
        if (render_objects[i].id == gfx->render_object_id) {
            render_objects[i].id = 0;
            render_objects[i].object = NULL;
            break;
        }
    }
    gfx->render_object_id = 0;
}

// ─────────────────────────────────────
// Trigger repaint callback in lua script
void pdlua_gfx_repaint(t_pdlua *o, int firsttime) {
    o->gfx.first_draw = firsttime;
    lua_getglobal(__L(), "pd");
    lua_getfield(__L(), -1, "_repaint");
    lua_pushlightuserdata(__L(), o);

    if (lua_pcall(__L(), 1, 0, 0)) {
        mylua_error(__L(), o, "repaint");
    }
    lua_pop(__L(), 1); /* pop the global "pd" */
    o->gfx.first_draw = 0;
}

// ─────────────────────────────────────
// Pass mouse events to lua script
void pdlua_gfx_mouse_event(t_pdlua *o, int x, int y, int type) {
    lua_getglobal(__L(), "pd");
    lua_getfield(__L(), -1, "_mouseevent");
    lua_pushlightuserdata(__L(), o);
    lua_pushinteger(__L(), x);
    lua_pushinteger(__L(), y);
    lua_pushinteger(__L(), type);

    if (lua_pcall(__L(), 4, 0, 0)) {
        mylua_error(__L(), o, "mouseevent");
    }

    lua_pop(__L(), 1); /* pop the global "pd" */
}

// ╭─────────────────────────────────────╮
// │           Mouse Movements           │
// ╰─────────────────────────────────────╯
// NOTE: We keep this 4 functions to avoid change pdlua.c
void pdlua_gfx_mouse_down(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 0);
}
void pdlua_gfx_mouse_up(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 1);
}
void pdlua_gfx_mouse_move(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 2);
}
void pdlua_gfx_mouse_drag(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 3);
}

// ─────────────────────────────────────
typedef struct _path_state {
    GuiPathElement elements[512];
    int count;
    float current_x, current_y;
} t_path_state;

// ─────────────────────────────────────
static t_pdlua_gfx *pop_graphics_context(lua_State *L) {
    t_pdlua_gfx *ctx = (t_pdlua_gfx *)luaL_checkudata(L, 1, "GraphicsContext");
    lua_remove(L, 1);
    return ctx;
}

// ─────────────────────────────────────
// Register functions with Lua
static const luaL_Reg gfx_lib[] = {{"set_size", set_size},
                                   {"get_size", get_size},
                                   {"start_paint", start_paint},
                                   {"end_paint", end_paint},
                                   {NULL, NULL}};

// ─────────────────────────────────────
static const luaL_Reg path_methods[] = {{"line_to", line_to},   {"quad_to", quad_to},
                                        {"cubic_to", cubic_to}, {"close", close_path},
                                        {"__gc", free_path},    {NULL, NULL}};

// ─────────────────────────────────────
// Register functions with Lua
static const luaL_Reg gfx_methods[] = {{"fill_ellipse", fill_ellipse},
                                       {"stroke_ellipse", stroke_ellipse},
                                       {"fill_rect", fill_rect},
                                       {"stroke_rect", stroke_rect},
                                       {"fill_rounded_rect", fill_rounded_rect},
                                       {"stroke_rounded_rect", stroke_rounded_rect},
                                       {"draw_line", draw_line},
                                       {"draw_text", draw_text},
                                       {"draw_svg", draw_svg},
                                       {"stroke_path", stroke_path},
                                       {"fill_path", fill_path},
                                       {"fill_all", fill_all},

                                       {"set_color", set_color},
                                       {"translate", translate},
                                       {"scale", scale},
                                       {"reset_transform", reset_transform},
                                       {NULL, NULL}};

// ─────────────────────────────────────
int pdlua_gfx_setup(lua_State *L) {
    lua_pushcfunction(L, start_path);
    lua_setglobal(L, "Path");

    luaL_newmetatable(L, "Path");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, path_methods, 0);

    luaL_newmetatable(L, "GraphicsContext");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, gfx_methods, 0);

    luaL_newlib(L, gfx_lib);
    lua_setglobal(L, "_gfx_internal");

    return 1;
}

// ─────────────────────────────────────
static int get_size(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }

    t_pdlua *obj = (t_pdlua *)lua_touserdata(L, 1);
    lua_pushnumber(L, (lua_Number)obj->gfx.width);
    lua_pushnumber(L, (lua_Number)obj->gfx.height);
    return 2;
}

// ─────────────────────────────────────
static void transform_size(t_pdlua_gfx *gfx, int *w, int *h) {
    for (int i = gfx->num_transforms - 1; i >= 0; i--) {
        if (gfx->transforms[i].type == SCALE) {
            *w *= gfx->transforms[i].x;
            *h *= gfx->transforms[i].y;
        }
    }
}

// ─────────────────────────────────────
static void transform_point(t_pdlua_gfx *gfx, int *x, int *y) {
    for (int i = gfx->num_transforms - 1; i >= 0; i--) {
        if (gfx->transforms[i].type == SCALE) {
            *x *= gfx->transforms[i].x;
            *y *= gfx->transforms[i].y;
        } else {
            *x += gfx->transforms[i].x;
            *y += gfx->transforms[i].y;
        }
    }
}

// ─────────────────────────────────────
static void transform_size_float(t_pdlua_gfx *gfx, float *w, float *h) {
    for (int i = gfx->num_transforms - 1; i >= 0; i--) {
        if (gfx->transforms[i].type == SCALE) {
            *w *= gfx->transforms[i].x;
            *h *= gfx->transforms[i].y;
        }
    }
}

// ─────────────────────────────────────
static void transform_point_float(t_pdlua_gfx *gfx, float *x, float *y) {
    for (int i = gfx->num_transforms - 1; i >= 0; i--) {
        if (gfx->transforms[i].type == SCALE) {
            *x *= gfx->transforms[i].x;
            *y *= gfx->transforms[i].y;
        } else {
            *x += gfx->transforms[i].x;
            *y += gfx->transforms[i].y;
        }
    }
}

// ─────────────────────────────────────
static void pdlua_gfx_clear(t_pdlua *obj, int layer, int removed) {
    if (!obj || !obj->gfx.render_object_id) return;
    if (removed || layer < 0) RemoveRenderObject(obj->gfx.render_object_id);
    else RemoveRenderLayer(obj->gfx.render_object_id, layer);
}

// ─────────────────────────────────────
static void gfx_displace(t_pdlua *x, t_glist *glist, int dx, int dy) {
    (void)dx;
    (void)dy;
    UpdateRenderObject(x->gfx.render_object_id, text_xpix((t_object *)x, glist),
                       text_ypix((t_object *)x, glist), x->gfx.width, x->gfx.height);
}

// ─────────────────────────────────────
static int gfx_initialize(t_pdlua *obj) {
    t_pdlua_gfx *gfx = &obj->gfx;
    snprintf(gfx->object_tag, 128, ".x%lx", (long)obj);
    gfx->object_tag[127] = '\0';
    gfx->order_tag[0] = '\0';
    gfx->object = obj;
    gfx->render_object_id = AllocateRenderObjectIdC();
    for (int i = 0; i < 256; ++i) {
        if (!render_objects[i].id) {
            render_objects[i].id = gfx->render_object_id;
            render_objects[i].object = obj;
            break;
        }
    }
    gfx->num_transforms = 0;
    gfx->num_layers = 0;
    gfx->layer_tags = NULL;
    gfx->current_layer_tag = gfx->object_tag;
    strcpy(gfx->current_color, "#000000");

    // char id[127];
    // sprintf(id, "Pd4WebInstance_%p", m_NewPdInstance);
    // Pd4WebClassBindStruct *x = (Pd4WebClassBindStruct *)getbytes(sizeof(Pd4WebClassBindStruct));
    // x->id = gensym(id);
    // pd_bind((t_pd *)x, x->id);
    // t_pdpy_objptr *x = (t_pdpy_objptr *)pd_findbyclass(s, pdpy_pyobjectout_class);

    pdlua_gfx_repaint(obj, 0);
    return 0;
}

// ─────────────────────────────────────
static int set_size(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }

    t_pdlua *obj = (t_pdlua *)lua_touserdata(L, 1);
    t_object *pdobj = (t_object *)obj;
    obj->gfx.width = luaL_checknumber(L, 2);
    obj->gfx.height = luaL_checknumber(L, 3);
    int x = obj->pd.te_xpix;
    int y = obj->pd.te_ypix;
    UpdateRenderObject(obj->gfx.render_object_id, x, y, obj->gfx.width, obj->gfx.height);
    pdlua_gfx_repaint(obj, 0);
    return 0;
}

// ─────────────────────────────────────
static int start_paint(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        lua_pushnil(L);
        return 1;
    }

    t_pdlua *obj = (t_pdlua *)lua_touserdata(L, 1);
    t_pdlua_gfx *gfx = &obj->gfx;

    // without object_tar
    if (gfx->object_tag[0] == '\0') {
        lua_pushnil(L);
        return 1;
    }

    // some layer was missed
    int layer = luaL_checkinteger(L, 2) - 1;
    gfx->current_layer = layer;
    if (layer > gfx->num_layers) {
        pdlua_gfx_repaint(obj, 0);
        lua_pushnil(L);
        return 1;
    }

    gfx->num_layers = layer + 1;

    gfx->first_draw = 0;

    int x = text_xpix((t_object *)obj, obj->canvas);
    int y = text_ypix((t_object *)obj, obj->canvas);

    ClearLayerCommand(gfx->render_object_id, gfx->current_layer, x, y, gfx->width, gfx->height);

    lua_pushlightuserdata(L, gfx);
    luaL_setmetatable(L, "GraphicsContext");
    return 1;
}

// ─────────────────────────────────────
static int end_paint(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;


    EndPaintLayerCommand(gfx->render_object_id, gfx->current_layer);
    return 0;
}

// ─────────────────────────────────────
static void get_bounds_args(lua_State *L, t_pdlua *obj, int *x1, int *y1, int *x2, int *y2) {
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    int x = luaL_checknumber(L, 1);
    int y = luaL_checknumber(L, 2);
    int w = luaL_checknumber(L, 3);
    int h = luaL_checknumber(L, 4);

    transform_point(&obj->gfx, &x, &y);
    transform_size(&obj->gfx, &w, &h);

    *x1 = x;
    *y1 = y;
    *x2 = w;
    *y2 = h;
}

// ─────────────────────────────────────
static int set_color(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    int r, g, b, a;
    if (lua_gettop(L) == 1) { // Single argument: parse as color ID instead of RGB
        int color_id = luaL_checknumber(L, 1);
        if (color_id != 1) {
            r = 255;
            g = 255;
            b = 255;
        } else {
            r = 0;
            g = 0;
            b = 0;
        }
    } else {
        r = luaL_checknumber(L, 1);
        g = luaL_checknumber(L, 2);
        b = luaL_checknumber(L, 3);
    }

    snprintf(gfx->current_color, 8, "#%02X%02X%02X", r, g, b);
    gfx->current_color[7] = '\0';

    return 0;
}

// ─────────────────────────────────────
static int fill_ellipse(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    GuiCommand cmd = {0};
    cmd.command = FILL_ELLIPSE;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);
    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = width;
    cmd.h = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);


    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int stroke_ellipse(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    GuiCommand cmd = {0};
    cmd.command = STROKE_ELLIPSE;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);
    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = width;
    cmd.h = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 5);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int fill_all(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = FILL_ALL;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int fill_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = FILL_RECT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = width;
    cmd.h = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);
    return 0;
}

// ─────────────────────────────────────
static int stroke_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = STROKE_RECT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = width;
    cmd.h = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 5);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int fill_rounded_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = FILL_ROUNDED_RECT;

    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = width;
    cmd.h = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.radius = luaL_checknumber(L, 5);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int stroke_rounded_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = STROKE_ROUNDED_RECT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = width;
    cmd.h = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.radius = luaL_checknumber(L, 5);
    cmd.line_width = luaL_checknumber(L, 6);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int draw_line(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = DRAW_LINE;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.x2 = width;
    cmd.y2 = height;
    cmd.line_width = luaL_checknumber(L, 5);
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static int draw_text(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    const char *text = luaL_checkstring(L, 1);
    int x = luaL_checknumber(L, 2);
    int y = luaL_checknumber(L, 3);
    int w = luaL_checknumber(L, 4);
    int font_height = luaL_checknumber(L, 5);

    transform_point(gfx, &x, &y);
    transform_size(gfx, &w, &font_height);

    GuiCommand cmd = {0};
    cmd.command = DRAW_TEXT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    size_t len = strlen(text);
    if (len > 1024) {
        snprintf(cmd.text, 1024, "Text is too long");
    } else {
        snprintf(cmd.text, 1024, "%s", text);
    }

    cmd.x1 = x;
    cmd.y1 = y;
    cmd.w = w;
    cmd.font_size = font_height;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);
    return 0;
}

// ─────────────────────────────────────
static int draw_svg(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    const char *svg = luaL_checkstring(L, 1);
    int x = luaL_checknumber(L, 2);
    int y = luaL_checknumber(L, 3);

    transform_point(gfx, &x, &y);

    GuiCommand cmd = {0};
    cmd.command = DRAW_SVG;

    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    cmd.svg = (char *)svg; // Copied into owned bounded transport storage by AddNewCommand().

    cmd.x1 = x;
    cmd.y1 = y;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);


    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);

    return 0;
}

static int transformed_path(t_pdlua_gfx *gfx, const t_path_state *path,
                            GuiPathElement *output) {
    for (int i = 0; i < path->count; ++i) {
        output[i] = path->elements[i];
        int points = 0;
        switch (output[i].verb) {
        case LUA_PATH_MOVE_TO:
        case LUA_PATH_LINE_TO: points = 1; break;
        case LUA_PATH_QUAD_TO: points = 2; break;
        case LUA_PATH_CUBIC_TO: points = 3; break;
        case LUA_PATH_CLOSE: points = 0; break;
        }
        for (int point = 0; point < points; ++point) {
            transform_point_float(gfx, &output[i].values[point * 2],
                                  &output[i].values[point * 2 + 1]);
        }
    }
    return path->count;
}

// ─────────────────────────────────────
static int stroke_path(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = STROKE_PATH;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (path->count < 2) {
        return 0;
    }

    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 2);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    GuiPathElement elements[512];
    cmd.path_elements = elements;
    cmd.path_element_count = transformed_path(gfx, path, elements);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);
    return 0;
}

// ─────────────────────────────────────
static int fill_path(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiCommand cmd = {0};
    cmd.command = FILL_PATH;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (path->count < 2) {
        return 0;
    }

    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;

    GuiPathElement elements[512];
    cmd.path_elements = elements;
    cmd.path_element_count = transformed_path(gfx, path, elements);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    AddNewCommand(gfx->render_object_id, gfx->current_layer, &cmd);
    return 0;
}

// ─────────────────────────────────────
static int translate(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    if (gfx->num_transforms >= 32) return luaL_error(L, "transform capacity exceeded");

    gfx->transforms[gfx->num_transforms].type = TRANSLATE;
    gfx->transforms[gfx->num_transforms].x = luaL_checknumber(L, 1);
    gfx->transforms[gfx->num_transforms].y = luaL_checknumber(L, 2);

    gfx->num_transforms++;
    return 0;
}

// ─────────────────────────────────────
static int scale(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    if (gfx->num_transforms >= 32) return luaL_error(L, "transform capacity exceeded");

    gfx->transforms[gfx->num_transforms].type = SCALE;
    gfx->transforms[gfx->num_transforms].x = luaL_checknumber(L, 1);
    gfx->transforms[gfx->num_transforms].y = luaL_checknumber(L, 2);

    gfx->num_transforms++;
    return 0;
}

// ─────────────────────────────────────
static int reset_transform(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    gfx->num_transforms = 0;
    return 0;
}

// ─────────────────────────────────────
static int append_path(t_path_state *path, enum LuaPathVerb verb, const float *values, int count) {
    if (path->count >= 512) return 0;
    GuiPathElement *element = &path->elements[path->count++];
    memset(element, 0, sizeof(*element));
    element->verb = verb;
    if (values && count > 0) memcpy(element->values, values, count * sizeof(float));
    return 1;
}

static int start_path(lua_State *L) {
    t_path_state *path = (t_path_state *)lua_newuserdata(L, sizeof(t_path_state));
    luaL_setmetatable(L, "Path");
    memset(path, 0, sizeof(*path));
    float values[2] = {luaL_checknumber(L, 1), luaL_checknumber(L, 2)};
    append_path(path, LUA_PATH_MOVE_TO, values, 2);
    path->current_x = values[0];
    path->current_y = values[1];
    return 1;
}

static int line_to(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    float values[2] = {luaL_checknumber(L, 2), luaL_checknumber(L, 3)};
    if (!append_path(path, LUA_PATH_LINE_TO, values, 2)) return luaL_error(L, "path capacity exceeded");
    path->current_x = values[0];
    path->current_y = values[1];
    return 0;
}

static int quad_to(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    float values[4] = {luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                       luaL_checknumber(L, 4), luaL_checknumber(L, 5)};
    if (!append_path(path, LUA_PATH_QUAD_TO, values, 4)) return luaL_error(L, "path capacity exceeded");
    path->current_x = values[2];
    path->current_y = values[3];
    return 0;
}

static int cubic_to(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    float values[6] = {luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                       luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                       luaL_checknumber(L, 6), luaL_checknumber(L, 7)};
    if (!append_path(path, LUA_PATH_CUBIC_TO, values, 6)) return luaL_error(L, "path capacity exceeded");
    path->current_x = values[4];
    path->current_y = values[5];
    return 0;
}

static int close_path(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (!append_path(path, LUA_PATH_CLOSE, NULL, 0)) return luaL_error(L, "path capacity exceeded");
    return 0;
}

static int free_path(lua_State *L) {
    (void)L;
    return 0;
}
