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

#include <config.h>

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

void pdlua_gfx_free(t_pdlua_gfx *gfx) {
    // TODO: need to free images
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
    float *path_segments;
    int num_path_segments;
    int num_path_segments_allocated;
    float path_start_x, path_start_y;
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
    // TODO:
}

// ─────────────────────────────────────
static void gfx_displace(t_pdlua *x, t_glist *glist, int dx, int dy) {
    // NOTE: No need to displace the object in the canvas since the because it will be fixed
}

// ─────────────────────────────────────
static int gfx_initialize(t_pdlua *obj) {
    t_pdlua_gfx *gfx = &obj->gfx;
    snprintf(gfx->object_tag, 128, ".x%lx", (long)obj);
    gfx->object_tag[127] = '\0';
    gfx->order_tag[0] = '\0';
    gfx->object = obj;
    gfx->transforms = NULL;
    gfx->num_transforms = 0;
    gfx->num_layers = 0;
    gfx->layer_tags = NULL;

    char id[127];
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
    pdlua_gfx_repaint(obj, 0);
    return 0;
}

// ─────────────────────────────────────
static int start_paint(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        lua_pushnil(L);
        printf("running tag is NULL\n");
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

    // malloc new layers
    int new_num_layers = layer + 1;
    if (gfx->layer_tags) {
        char **new_tags = resizebytes(gfx->layer_tags, sizeof(char *) * gfx->num_layers,
                                      sizeof(char *) * new_num_layers);
        if (!new_tags) {
            lua_pushnil(L);
            return 1;
        }
        gfx->layer_tags = new_tags;
    } else {
        gfx->layer_tags = getbytes(sizeof(char *) * new_num_layers);
        if (!gfx->layer_tags) {
            printf("failed to alloc\n");
            lua_pushnil(L);
            return 1;
        }
    }

    // Initialize any new entries to NULL
    for (int i = gfx->num_layers; i < new_num_layers; ++i) {
        gfx->layer_tags[i] = NULL;
    }

    gfx->num_layers = new_num_layers;
    gfx->layer_tags[layer] = getbytes(64);
    if (!gfx->layer_tags[layer]) {
        printf("failed to alloc for tags\n");
        lua_pushnil(L);
        return 1;
    }

    gfx->first_draw = 0;

    int x = text_xpix((t_object *)obj, obj->canvas);
    int y = text_ypix((t_object *)obj, obj->canvas);

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    ClearLayerCommand(obj_layer_id, gfx->current_layer, x, y, gfx->width, gfx->height);

    lua_pushlightuserdata(L, gfx);
    luaL_setmetatable(L, "GraphicsContext");
    return 1;
}

// ─────────────────────────────────────
static int end_paint(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);

    EndPaintLayerCommand(obj_layer_id, gfx->current_layer);
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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);

    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);
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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

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

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);
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

    size_t len = strlen(svg);
    cmd.svg = malloc(len + 1);
    if (!cmd.svg) {
        return luaL_error(L, "Out of memory allocating SVG");
    }
    memcpy(cmd.svg, svg, len + 1);

    cmd.x1 = x;
    cmd.y1 = y;
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    char obj_layer_id[64];
    snprintf(obj_layer_id, sizeof(obj_layer_id), "layer_%p", obj);

    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);

    return 0;
}

// ─────────────────────────────────────
static uint64_t pdlua_image_hash(unsigned char *str) {
    uint64_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

// ─────────────────────────────────────
uint32_t pdlua_float_hash(float f) {
    union {
        float f;
        uint32_t i;
    } u;
    u.f = f;
    return u.i * 0x9E3779B9;
}

// ─────────────────────────────────────
static char *pdlua_base64_encode(const unsigned char *data, size_t input_length) {

    static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

    static int mod_table[] = {0, 2, 1};

    size_t output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(output_length + 1);
    if (encoded_data == NULL) {
        return NULL;
    }

    for (size_t i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }

    encoded_data[output_length] = '\0';
    return encoded_data;
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
    if (path->num_path_segments < 3) {
        return 0;
    }

    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 2);
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    cmd.path_coords = malloc(path->num_path_segments * 2 * sizeof(t_float));
    cmd.path_size = path->num_path_segments;
    for (int i = 0; i < path->num_path_segments; i++) {
        float x = path->path_segments[i * 2], y = path->path_segments[i * 2 + 1];
        transform_point_float(gfx, &x, &y);
        cmd.path_coords[i * 2] = x;
        cmd.path_coords[i * 2 + 1] = y;
    }

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);
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
    if (path->num_path_segments < 3) {
        return 0;
    }

    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;

    cmd.path_coords = malloc(path->num_path_segments * 2 * sizeof(t_float));
    cmd.path_size = path->num_path_segments;
    for (int i = 0; i < path->num_path_segments; i++) {
        float x = path->path_segments[i * 2], y = path->path_segments[i * 2 + 1];
        transform_point_float(gfx, &x, &y);
        cmd.path_coords[i * 2] = x;
        cmd.path_coords[i * 2 + 1] = y;
    }
    cmd.objx = text_xpix((t_object *)obj, obj->canvas);
    cmd.objy = text_ypix((t_object *)obj, obj->canvas);

    char obj_layer_id[64];
    snprintf(obj_layer_id, 64, "layer_%p", obj);
    AddNewCommand(obj_layer_id, gfx->current_layer, &cmd);
    return 0;
}

// ─────────────────────────────────────
static int translate(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    if (gfx->num_transforms == 0) {
        gfx->transforms = getbytes(sizeof(gfx_transform));

    } else {
        gfx->transforms = resizebytes(gfx->transforms, gfx->num_transforms * sizeof(gfx_transform),
                                      (gfx->num_transforms + 1) * sizeof(gfx_transform));
    }

    gfx->transforms[gfx->num_transforms].type = TRANSLATE;
    gfx->transforms[gfx->num_transforms].x = luaL_checknumber(L, 1);
    gfx->transforms[gfx->num_transforms].y = luaL_checknumber(L, 2);

    gfx->num_transforms++;
    return 0;
}

// ─────────────────────────────────────
static int scale(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    gfx->transforms = resizebytes(gfx->transforms, gfx->num_transforms * sizeof(gfx_transform),
                                  (gfx->num_transforms + 1) * sizeof(gfx_transform));

    gfx->transforms[gfx->num_transforms].type = SCALE;
    gfx->transforms[gfx->num_transforms].x = luaL_checknumber(L, 1);
    gfx->transforms[gfx->num_transforms].y = luaL_checknumber(L, 2);

    gfx->num_transforms++;
    return 0;
}

// ─────────────────────────────────────
static int reset_transform(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    freebytes(gfx->transforms, gfx->num_transforms * sizeof(gfx_transform));
    gfx->transforms = NULL;
    gfx->num_transforms = 0;
    return 0;
}

// ─────────────────────────────────────
static void add_path_segment(t_path_state *path, float x, float y) {
    int path_segment_space = (path->num_path_segments + 1) * 2;
    int old_size = path->num_path_segments_allocated;
    int new_size = MAX(path_segment_space, path->num_path_segments_allocated);
    if (!path->num_path_segments_allocated) {
        path->path_segments = (float *)getbytes(new_size * sizeof(float));
    } else {
        // BUG: Futex error (resize memory from the Audio Worklet)
        path->path_segments = (float *)resizebytes(path->path_segments, old_size * sizeof(float),
                                                   new_size * sizeof(float));
    }

    path->num_path_segments_allocated = new_size;

    path->path_segments[path->num_path_segments * 2] = x;
    path->path_segments[path->num_path_segments * 2 + 1] = y;
    path->num_path_segments++;
}

// ─────────────────────────────────────
static int start_path(lua_State *L) {
    t_path_state *path = (t_path_state *)lua_newuserdata(L, sizeof(t_path_state));
    luaL_setmetatable(L, "Path");

    path->num_path_segments = 0;
    path->num_path_segments_allocated = 0;
    path->path_start_x = luaL_checknumber(L, 1);
    path->path_start_y = luaL_checknumber(L, 2);

    add_path_segment(path, path->path_start_x, path->path_start_y);
    return 1;
}

// ─────────────────────────────────────
// Function to add a line to the current path
static int line_to(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    add_path_segment(path, x, y);
    return 0;
}

// ─────────────────────────────────────
static int quad_to(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    float x2 = luaL_checknumber(L, 2);
    float y2 = luaL_checknumber(L, 3);
    float x3 = luaL_checknumber(L, 4);
    float y3 = luaL_checknumber(L, 5);

    float x1 =
        path->num_path_segments > 0 ? path->path_segments[(path->num_path_segments - 1) * 2] : x2;
    float y1 = path->num_path_segments > 0
                   ? path->path_segments[(path->num_path_segments - 1) * 2 + 1]
                   : y2;

    // heuristic for deciding the number of lines in our bezier curve
    float dx = x3 - x1;
    float dy = y3 - y1;
    float distance = sqrtf(dx * dx + dy * dy);
    float resolution = MAX(10.0f, distance);

    // Get the last point
    float t = 0.0;
    while (t <= 1.0) {
        t += 1.0 / resolution;

        // Calculate quadratic bezier curve as points (source:
        // https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
        float x = (1.0f - t) * (1.0f - t) * x1 + 2.0f * (1.0f - t) * t * x2 + t * t * x3;
        float y = (1.0f - t) * (1.0f - t) * y1 + 2.0f * (1.0f - t) * t * y2 + t * t * y3;
        add_path_segment(path, x, y);
    }

    return 0;
}

// ─────────────────────────────────────
static int cubic_to(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    float x2 = luaL_checknumber(L, 2);
    float y2 = luaL_checknumber(L, 3);
    float x3 = luaL_checknumber(L, 4);
    float y3 = luaL_checknumber(L, 5);
    float x4 = luaL_checknumber(L, 6);
    float y4 = luaL_checknumber(L, 7);

    float x1 =
        path->num_path_segments > 0 ? path->path_segments[(path->num_path_segments - 1) * 2] : x2;
    float y1 = path->num_path_segments > 0
                   ? path->path_segments[(path->num_path_segments - 1) * 2 + 1]
                   : y2;

    // heuristic for deciding the number of lines in our bezier curve
    float dx = x3 - x1;
    float dy = y3 - y1;
    float distance = sqrtf(dx * dx + dy * dy);
    float resolution = MAX(10.0f, distance);

    // Get the last point
    float t = 0.0;
    while (t <= 1.0) {
        t += 1.0 / resolution;

        // Calculate cubic bezier curve as points (source:
        // https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
        float x = (1 - t) * (1 - t) * (1 - t) * x1 + 3 * (1 - t) * (1 - t) * t * x2 +
                  3 * (1 - t) * t * t * x3 + t * t * t * x4;
        float y = (1 - t) * (1 - t) * (1 - t) * y1 + 3 * (1 - t) * (1 - t) * t * y2 +
                  3 * (1 - t) * t * t * y3 + t * t * t * y4;

        add_path_segment(path, x, y);
    }

    return 0;
}

// ─────────────────────────────────────
// Function to close the current path
static int close_path(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    add_path_segment(path, path->path_start_x, path->path_start_y);
    return 0;
}

// ─────────────────────────────────────
static int free_path(lua_State *L) {
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    freebytes(path->path_segments, path->num_path_segments_allocated * sizeof(int));
    return 0;
}
