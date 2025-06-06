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

#include <GLES3/gl3.h>
#include <nanovg.h>
#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg_gl.h> // or nanovg_gl3.h depending on your setup

#include <config.h>
#include <g_canvas.h>
#include <m_pd.h>
#include <s_stuff.h>

// PD4WEB
enum LuaGuiCommands {
    FILL_ELLIPSE = 0,
    STROKE_ELLIPSE,
    FILL_RECT,
    STROKE_RECT,
    FILL_ROUNDED_RECT,
    STROKE_ROUNDED_RECT,
    DRAW_LINE,
    DRAW_TEXT,
    // DRAW_SVG, // TODO: implement this
    STROKE_PATH,
    FILL_PATH,
    FILL_ALL,
    CLEAR_CANVAS,
};

typedef struct {
    enum LuaGuiCommands command;
    int drawed;
    char current_color[128];
    char layer_id[128];
    // char *svg_text;
    float x;
    float y;
    float scale;
    float width;
    float height;
    float w;
    float h;
    float line_width;
    float radius;
    float x1;
    float y1;
    float x2;
    float y2;
    char *text; // TODO: Fix this
    float font_size;
    float stroke_width;

    float canvas_width;
    float canvas_height;

    float *path_coords;
    int path_size;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    GLuint Shader;
} GuiValues;

#define PDLUA_MAX_COMMANDS 512
static GuiValues commandQueue[PDLUA_MAX_COMMANDS];
static int queueStart = 0;
static int queueEnd = 0;

// ╭─────────────────────────────────────╮
// │  This will be called from the main  │
// │   thread inside pd4web main loop    │
// ╰─────────────────────────────────────╯
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE create_webgl_context_for_layer(char *tag) {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = true;
    attr.depth = false;
    attr.stencil = true;
    attr.antialias = true;
    attr.premultipliedAlpha = true; // Recommended
    attr.preserveDrawingBuffer = false;
    attr.failIfMajorPerformanceCaveat = false;

    attr.majorVersion = 2;
    attr.minorVersion = 0;

    attr.enableExtensionsByDefault = true; // Enable extensions automatically
    attr.explicitSwapControl = false;
    attr.renderViaOffscreenBackBuffer = false;

    char selector[128];
    pd_snprintf(selector, 128, "#%s", tag);

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(selector, &attr);
    if (ctx == 0) {
        fprintf(stderr, "Failed to create WebGL context for canvas: %s\n", selector);
        return 0;
    }

    if (emscripten_webgl_make_context_current(ctx) != EMSCRIPTEN_RESULT_SUCCESS) {
        emscripten_webgl_destroy_context(ctx);
        fprintf(stderr, "Failed to make WebGL context current for canvas: %s\n", selector);
        return 0;
    }

    return ctx;
}

// ─────────────────────────────────────
int enqueue_command(GuiValues *cmd) {
    int next = (queueEnd + 1) % PDLUA_MAX_COMMANDS;
    if (next == queueStart) {
        return 0;
    }
    commandQueue[queueEnd] = *cmd;
    queueEnd = next;
    return 1; // sucesso
}

// ─────────────────────────────────────
int dequeue_command(GuiValues *out) {
    if (queueStart == queueEnd) {
        return 0;
    }
    *out = commandQueue[queueStart];
    queueStart = (queueStart + 1) % PDLUA_MAX_COMMANDS;
    return 1;
}

// ─────────────────────────────────────
static void hex_to_rgb_normalized(const char *hex, float *r, float *g, float *b) {
    if (hex[0] == '#') {
        hex++;
    }

    char rs[3] = {hex[0], hex[1], '\0'};
    char gs[3] = {hex[2], hex[3], '\0'};
    char bs[3] = {hex[4], hex[5], '\0'};

    int ri = (int)strtol(rs, NULL, 16);
    int gi = (int)strtol(gs, NULL, 16);
    int bi = (int)strtol(bs, NULL, 16);

    *r = ri / 255.0f;
    *g = gi / 255.0f;
    *b = bi / 255.0f;
}

// ╭─────────────────────────────────────╮
// │         LAYERS AND CONTEXTS         │
// ╰─────────────────────────────────────╯
#define MAX_LAYERS 128

typedef struct {
    const char *layer_id; // apontando para string persistente
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_ctx;
    NVGcontext *vg;
    int in_use;
} LayerCtx;

static LayerCtx layer_contexts[MAX_LAYERS];

// ─────────────────────────────────────
LayerCtx *get_layer_ctx(const char *layer_id, float canvas_width, float canvas_height) {
    for (int i = 0; i < MAX_LAYERS; i++) {
        if (layer_contexts[i].in_use && strcmp(layer_contexts[i].layer_id, layer_id) == 0) {
            emscripten_webgl_make_context_current(layer_contexts[i].webgl_ctx);
            return &layer_contexts[i];
        }
    }
    for (int i = 0; i < MAX_LAYERS; i++) {
        if (!layer_contexts[i].in_use) {
            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer((char *)layer_id);
            if (!ctx) {
                fprintf(stderr, "Failed to create ctx\n");
                return NULL;
            }
            emscripten_webgl_make_context_current(ctx);
            NVGcontext *vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
            if (!vg) {
                fprintf(stderr, "Failed to create NVG context\n");
                emscripten_webgl_destroy_context(ctx);
                return NULL;
            }

            layer_contexts[i].layer_id = strdup(layer_id); // cuidado com o ciclo de vida da string
            layer_contexts[i].webgl_ctx = ctx;
            layer_contexts[i].vg = vg;
            layer_contexts[i].in_use = 1;
            int font = nvgCreateFont(vg, "sans", "dejavu.ttf");
            if (font == -1) {
                fprintf(stderr, "Failed to create font\n");
                return NULL;
            }
            return &layer_contexts[i];
        }
    }
    return NULL;
}

// ─────────────────────────────────────
static bool isDrawing = 0;
int pd4weblua_draw() {
    if (isDrawing) {
        return 0;
    }
    GuiValues cmd;
    float devicePixelRatio = PD4WEB_PATCH_ZOOM * emscripten_get_device_pixel_ratio();
    while (dequeue_command(&cmd)) {

        switch (cmd.command) {
        case FILL_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                printf("Failed to get layer context for layer %s\n", cmd.layer_id);
                break;
            }
            NVGcontext *vg = ctx->vg;
            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);

            nvgBeginPath(vg);
            nvgFillColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgRect(vg, x, y, width, height);
            nvgFill(vg);

            nvgEndFrame(vg);
            break;
        }
        case STROKE_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float thickness = cmd.line_width;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;

            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);

            nvgBeginPath(vg);
            nvgStrokeWidth(vg, thickness);
            nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgRect(vg, x, y, width, height);
            nvgStroke(vg);
            nvgEndFrame(vg);
            break;
        }
        case FILL_ELLIPSE: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;
            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);
            nvgBeginPath(vg);
            float cx = x + width * 0.5f;
            float cy = y + height * 0.5f;
            float rx = width * 0.5f;
            float ry = height * 0.5f;
            nvgFillColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgEllipse(vg, cx, cy, rx, ry);
            nvgFill(vg);
            nvgEndFrame(vg);
            break;
        }
        case STROKE_ELLIPSE: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float line_width = cmd.line_width;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;

            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);

            nvgBeginPath(vg);
            float cx = x + width * 0.5f;
            float cy = y + height * 0.5f;
            float rx = width * 0.5f;
            float ry = height * 0.5f;

            nvgStrokeWidth(vg, line_width);
            nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgEllipse(vg, cx, cy, rx, ry);
            nvgStroke(vg);
            nvgEndFrame(vg);
            break;
        }

        case FILL_ROUNDED_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float radius = cmd.radius;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            float canvas_width = cmd.canvas_width;
            float canvas_height = cmd.canvas_height;
            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;
            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);
            nvgBeginPath(vg);
            nvgFillColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgRoundedRect(vg, x, y, width, height, radius);
            nvgFill(vg);
            nvgEndFrame(vg);
            break;
        }

        case STROKE_ROUNDED_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float radius = cmd.radius;
            float thickness = cmd.line_width;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            float canvas_width = cmd.canvas_width;
            float canvas_height = cmd.canvas_height;

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;
            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);
            nvgBeginPath(vg);
            nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgRoundedRect(vg, x, y, width, height, radius);
            nvgStrokeWidth(vg, thickness);
            nvgStroke(vg);
            nvgEndFrame(vg);
            break;
        }

        case DRAW_LINE: {
            float x1 = cmd.x1;
            float y1 = cmd.y1;
            float x2 = cmd.x2;
            float y2 = cmd.y2;
            float line_width = cmd.line_width;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            float canvas_width = cmd.canvas_width;
            float canvas_height = cmd.canvas_height;
            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;
            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);

            nvgBeginPath(vg);
            nvgStrokeWidth(vg, line_width);
            nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgMoveTo(vg, x1, y1);
            nvgLineTo(vg, x2, y2);
            nvgStroke(vg);
            nvgEndFrame(vg);
            break;
        }

        case STROKE_PATH: {
            float line_width = cmd.line_width / PD4WEB_PATCH_ZOOM;
            float *coords = cmd.path_coords;
            int coords_len = cmd.path_size;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            float canvas_width = cmd.canvas_width;
            float canvas_height = cmd.canvas_height;

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;

            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);
            nvgBeginPath(vg);
            nvgStrokeWidth(vg, line_width);
            nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0f));

            if (coords_len >= 2) {
                nvgMoveTo(vg, coords[0], coords[1]);
                for (int i = 1; i < coords_len; i++) {
                    nvgLineTo(vg, coords[i * 2], coords[i * 2 + 1]);
                }
            }
            nvgStroke(vg);
            nvgEndFrame(vg);
            free(cmd.path_coords);
            break;
        }
        case FILL_PATH: {
            float *coords = cmd.path_coords;
            int coords_len = cmd.path_size;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            float canvas_width = cmd.canvas_width;
            float canvas_height = cmd.canvas_height;

            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            NVGcontext *vg = ctx->vg;
            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);
            nvgBeginPath(vg);
            if (coords_len >= 2) {
                nvgMoveTo(vg, coords[0], coords[1]);
                for (int i = 1; i < coords_len; i++) {
                    nvgLineTo(vg, coords[i * 2], coords[i * 2 + 1]);
                }
                nvgClosePath(vg);
            }
            nvgFillColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgFill(vg);
            nvgEndFrame(vg);
            free(cmd.path_coords);
            break;
        }
        case FILL_ALL: {
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx) {
                break;
            }
            glClearColor(r, g, b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            break;
        }
        case DRAW_TEXT: {
            const char *text = cmd.text;
            float x = cmd.x1;
            float y = cmd.y1;
            float maxWidth = cmd.width;
            float fontSize = cmd.font_size;
            float r, g, b;

            if (!text || !cmd.layer_id || maxWidth <= 0 || fontSize <= 0) {
                break; // Invalid parameters
            }

            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            LayerCtx *ctx = get_layer_ctx(cmd.layer_id, cmd.canvas_width, cmd.canvas_height);
            if (!ctx || !ctx->vg) {
                break;
            }

            NVGcontext *vg = ctx->vg;

            nvgBeginFrame(vg, cmd.canvas_width, cmd.canvas_height, devicePixelRatio);
            nvgFontSize(vg, fontSize);
            nvgFontFace(vg, "sans");
            nvgFillColor(vg, nvgRGBAf(r, g, b, 1.0f));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

            // Use nvgTextBox with a NULL end pointer to draw wrapped text
            nvgTextBox(vg, x, y, maxWidth, text, NULL);

            nvgEndFrame(vg);
            break;
        }
        }
    }
    return 0;
}

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

    // int x = text_xpix((t_object *)o, o->canvas);
    // int y = text_ypix((t_object *)o, o->canvas);

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

// ─────────────────────────────────────
// Pass mouse events to lua script (but easier to understand)
void pdlua_gfx_mouse_down(t_pdlua *o, int x, int y) { pdlua_gfx_mouse_event(o, x, y, 0); }
void pdlua_gfx_mouse_up(t_pdlua *o, int x, int y) { pdlua_gfx_mouse_event(o, x, y, 1); }
void pdlua_gfx_mouse_move(t_pdlua *o, int x, int y) { pdlua_gfx_mouse_event(o, x, y, 2); }
void pdlua_gfx_mouse_drag(t_pdlua *o, int x, int y) { pdlua_gfx_mouse_event(o, x, y, 3); }

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
                                       // {"draw_svg", draw_svg}, // TODO:
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
        return 1;
    }
    isDrawing = 1;

    t_pdlua *obj = (t_pdlua *)lua_touserdata(L, 1);
    t_pdlua_gfx *gfx = &obj->gfx;

    if (gfx->object_tag[0] == '\0') {
        lua_pushnil(L);
        isDrawing = 0;
        return 1;
    }

    int layer = luaL_checkinteger(L, 2) - 1;

    if (layer > gfx->num_layers) {
        pdlua_gfx_repaint(obj, 0);
        lua_pushnil(L);
        isDrawing = 0;
        return 1;
    }

    int new_num_layers = layer + 1;
    if (gfx->layer_tags) {
        gfx->layer_tags = resizebytes(gfx->layer_tags, sizeof(char *) * gfx->num_layers,
                                      sizeof(char *) * new_num_layers);
    } else {
        gfx->layer_tags = getbytes(sizeof(char *));
    }

    if (gfx->first_draw || layer >= gfx->num_layers) {
        gfx->layer_tags[layer] = getbytes(64);
        snprintf(gfx->layer_tags[layer], 64, "layer_%p_%d", obj, layer);
        gfx->num_layers = new_num_layers;
        int x = text_xpix((t_object *)obj, obj->canvas);
        int y = text_ypix((t_object *)obj, obj->canvas);
        EM_ASM(
            {
                let layer_id = UTF8ToString($0);
                let x_pos = $1;
                let y_pos = $2;
                let width = $3;
                let height = $4;
                let zoom = $5;
                let dpr = window.devicePixelRatio || 1;
                let scale = zoom * dpr;

                const container = document.getElementById("Pd4WebPatchDiv");
                let item = document.getElementById(layer_id);
                if (!item) {
                    item = document.createElement("canvas");
                    container.appendChild(item);
                }

                item.id = layer_id;
                item.width = width * scale;
                item.height = height * scale;

                item.style.width = (width * zoom) + "px";
                item.style.height = (height * zoom) + "px";

                item.style.position = "absolute";
                item.style.border = zoom + "px solid #000";
                item.style.boxShadow = "1px 1px 2px rgba(0, 0, 0, 0.2)";
                item.style.left = (x_pos * zoom) + "px";
                item.style.top = (y_pos * zoom) + "px";
            },
            gfx->layer_tags[layer], x, y, gfx->width, gfx->height, PD4WEB_PATCH_ZOOM);

        gfx->first_draw = 0;
    }
    gfx->current_layer_tag = gfx->layer_tags[layer];
    lua_pushlightuserdata(L, gfx);
    luaL_setmetatable(L, "GraphicsContext");
    return 1;
}

// ─────────────────────────────────────
static int end_paint(lua_State *L) {
    // TODO: What to do here?
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = (t_pdlua *)gfx->object;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    isDrawing = 0;

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
    GuiValues cmd = {0};
    cmd.command = FILL_ELLIPSE;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);
    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.width = width;
    cmd.height = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }
    return 0;
}

// ─────────────────────────────────────
static int stroke_ellipse(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    GuiValues cmd = {0};
    cmd.command = STROKE_ELLIPSE;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);
    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.width = width;
    cmd.height = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 5);
    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }

    return 0;
}

// ─────────────────────────────────────
static int fill_all(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    GuiValues cmd = {0};
    cmd.command = FILL_ALL;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }
    return 0;
}

// ─────────────────────────────────────
static int fill_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiValues cmd = {0};
    cmd.command = FILL_RECT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.width = width;
    cmd.height = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }
    return 0;
}

// ─────────────────────────────────────
static int stroke_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    t_pdlua *obj = gfx->object;

    GuiValues cmd = {0};
    cmd.command = STROKE_RECT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.width = width;
    cmd.height = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 5);

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }

    return 0;
}

// ─────────────────────────────────────
static int fill_rounded_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiValues cmd = {0};
    cmd.command = FILL_ROUNDED_RECT;

    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.width = width;
    cmd.height = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.radius = luaL_checknumber(L, 5);

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }

    return 0;
}

// ─────────────────────────────────────
static int stroke_rounded_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiValues cmd = {0};
    cmd.command = STROKE_ROUNDED_RECT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    int x, y, width, height;
    get_bounds_args(L, obj, &x, &y, &width, &height);
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.width = width;
    cmd.height = height;
    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.radius = luaL_checknumber(L, 5);
    cmd.line_width = luaL_checknumber(L, 6);

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }

    return 0;

    return 0;
}

// ─────────────────────────────────────
static int draw_line(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiValues cmd = {0};
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
    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }
    return 0;
}

// ─────────────────────────────────────
static int draw_text(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues cmd = {0};
    cmd.command = DRAW_TEXT;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    const char *text = luaL_checkstring(L, 1);
    size_t len = strlen(text) + 1;
    cmd.text = (char *)malloc(len);
    if (cmd.text == NULL) {
        luaL_error(L, "Error allocating memory for text");
        return 0;
    }
    strncpy(cmd.text, text, len);
    cmd.x1 = luaL_checknumber(L, 2);
    cmd.y1 = luaL_checknumber(L, 3);
    cmd.width = luaL_checknumber(L, 4);
    cmd.font_size = luaL_checknumber(L, 5);

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }

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

    GuiValues cmd = {0};
    cmd.command = STROKE_PATH;
    strncpy(cmd.current_color, gfx->current_color, sizeof(cmd.current_color) - 1);
    strncpy(cmd.layer_id, gfx->current_layer_tag, sizeof(cmd.layer_id) - 1);

    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (path->num_path_segments < 3) {
        return 0;
    }

    cmd.canvas_width = gfx->width;
    cmd.canvas_height = gfx->height;
    cmd.line_width = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;

    cmd.path_coords = malloc(path->num_path_segments * 2 * sizeof(t_float));
    cmd.path_size = path->num_path_segments;
    for (int i = 0; i < path->num_path_segments; i++) {
        float x = path->path_segments[i * 2], y = path->path_segments[i * 2 + 1];
        transform_point_float(gfx, &x, &y);
        cmd.path_coords[i * 2] = x;
        cmd.path_coords[i * 2 + 1] = y;
    }

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }
    return 0;
}

// ─────────────────────────────────────
static int fill_path(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;

    GuiValues cmd = {0};
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

    if (!enqueue_command(&cmd)) {
        return luaL_error(L, "Command queue full");
    }
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
