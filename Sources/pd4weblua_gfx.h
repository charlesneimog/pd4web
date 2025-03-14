/** @file pdlua_gfx.h
 *  @brief pdlua_gfx -- an extension to pdlua that allows GUI rendering and interaction in pure-data
 * and plugdata
 *  @author Timothy Schoen <timschoen123@gmail.com>
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

#include <lua.h>
#include <lualib.h>

#include <emscripten.h>

#include <config.h>
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
    DRAW_SVG, // TODO: implement this
    STROKE_PATH,
    FILL_PATH,
    FILL_ALL,
    CLEAR_CANVAS,
};

typedef struct {
    enum LuaGuiCommands command;
    int drawed;
    const char current_color[64];
    const char layer_id[64];
    const char *svg_text;
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
    const char *text; // TODO: Fix this
    float font_height;
    float stroke_width;

    float *path_coords;
    int path_size;
} GuiValues;

typedef struct {
    int size;
    int i;
    GuiValues *values;
} Pd4WebLuaGuiQueue;

static Pd4WebLuaGuiQueue *LuaGuiQueue = NULL;

// ╭─────────────────────────────────────╮
// │  This will be called from the main  │
// │   thread inside pd4web main loop    │
// ╰─────────────────────────────────────╯
int pd4weblua_draw() {

    for (int i = 0; i < LuaGuiQueue->size; i++) {
        int command = LuaGuiQueue->values[i].command;
        GuiValues v = LuaGuiQueue->values[i];
        if (v.drawed) {
            continue;
        }

        switch (command) {
        case CLEAR_CANVAS:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var layer_id = UTF8ToString($0);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("fill_ellipse: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.clearRect(0, 0, item.width, item.height);
                },
                v.layer_id);

        case FILL_ELLIPSE:
            MAIN_THREAD_ASYNC_EM_ASM(
                // clang-format off
                {
                    var layer_id = UTF8ToString($0);
                    var color = UTF8ToString($1);
                    let x = $2;
                    let y = $3;
                    let w = $4;
                    let h = $5;

                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("fill_ellipse: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');

                    let centerX = x + w / 2;
                    let centerY = y + h / 2;

                    ctx.fillStyle = color;
                    ctx.beginPath();
                    ctx.ellipse(centerX, centerY, w / 2, h / 2, 0, 0, 2 * Math.PI);
                    ctx.fill();
                },
                // clang-format on
                v.layer_id, v.current_color, v.x, v.y, v.w, v.h);
            break;
        case STROKE_ELLIPSE:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var layer_id = UTF8ToString($0);
                    var color = UTF8ToString($1);
                    let x = $2;
                    let y = $3;
                    let w = $4;
                    let h = $5;
                    let line_width = $6;

                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("stroke_ellipse: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');

                    let centerX = x + w / 2;
                    let centerY = y + h / 2;

                    ctx.strokeStyle = color;
                    ctx.lineWidth = line_width;
                    ctx.beginPath();
                    ctx.ellipse(centerX, centerY, w / 2, h / 2, 0, 0, 2 * Math.PI);
                    ctx.stroke();
                },
                v.layer_id, v.current_color, v.x, v.y, v.w, v.h, v.line_width);
            break;
        case FILL_RECT:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("fill_rect: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.fillStyle = color;
                    ctx.fillRect($2, $3, $4, $5);
                },
                v.current_color, v.layer_id, v.x, v.y, v.w, v.h);
            break;
        case STROKE_RECT:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("stroke_rect: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.strokeStyle = color;
                    ctx.lineWidth = $6;
                    ctx.strokeRect($2, $3, $4, $5);
                    ctx.lineJoin = "bevel";
                },
                v.current_color, v.layer_id, v.x, v.y, v.w, v.h, v.line_width);
            break;
        case FILL_ROUNDED_RECT:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("fill_rounded_rect: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.fillStyle = color;
                    // Create rounded rectangle path
                    ctx.beginPath();
                    ctx.moveTo($2 + $6, $3);
                    ctx.lineTo($2 + $4 - $6, $3);
                    ctx.arcTo($2 + $4, $3, $2 + $4, $3 + $6, $6);
                    ctx.lineTo($2 + $4, $3 + $5 - $6);
                    ctx.arcTo($2 + $4, $3 + $5, $2 + $4 - $6, $3 + $5, $6);
                    ctx.lineTo($2 + $6, $3 + $5);
                    ctx.arcTo($2, $3 + $5, $2, $3 + $5 - $6, $6);
                    ctx.lineTo($2, $3 + $6);
                    ctx.arcTo($2, $3, $2 + $6, $3, $6);
                    ctx.closePath();
                    ctx.fill();
                },
                v.current_color, v.layer_id, v.x, v.y, v.w, v.h, v.radius);
            break;
        case STROKE_ROUNDED_RECT:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("stroke_rounded_rect: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.strokeStyle = color;
                    // Create rounded rectangle path
                    ctx.beginPath();
                    ctx.moveTo($2 + $6, $3);
                    ctx.lineTo($2 + $4 - $6, $3);
                    ctx.arcTo($2 + $4, $3, $2 + $4, $3 + $6, $6);
                    ctx.lineTo($2 + $4, $3 + $5 - $6);
                    ctx.arcTo($2 + $4, $3 + $5, $2 + $4 - $6, $3 + $5, $6);
                    ctx.lineTo($2 + $6, $3 + $5);
                    ctx.arcTo($2, $3 + $5, $2, $3 + $5 - $6, $6);
                    ctx.lineTo($2, $3 + $6);
                    ctx.arcTo($2, $3, $2 + $6, $3, $6);
                    ctx.closePath();
                    ctx.stroke();
                },
                v.current_color, v.layer_id, v.x, v.y, v.w, v.h, v.radius);
            break;
        case DRAW_LINE:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("draw_line: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.strokeStyle = color;
                    ctx.lineWidth = $6;
                    ctx.lineJoin = "bevel";

                    // Start drawing the line
                    ctx.beginPath();
                    ctx.moveTo($2, $3);
                    ctx.lineTo($4, $5);
                    ctx.stroke();
                },
                v.current_color, v.layer_id, v.x1, v.y1, v.x2, v.y2, v.line_width);
            break;
        case DRAW_TEXT:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    // clang-format off
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var text = UTF8ToString($2);
                    var x = $3;
                    var y = $4;
                    var maxWidth = $5;
                    var fontSize = $6;

                    var item = document.getElementById(layer_id);
                    if (!item) {
                        console.log("draw_text: canvas not found");
                        return;
                    }

                    const ctx = item.getContext('2d');
                    ctx.fillStyle = color;
                    ctx.font = fontSize + "px Arial";
                    ctx.textAlign = "left";
                    ctx.textBaseline = "top";

                    // Pure Data-style text wrapping algorithm
                    function pdTextWrap(context, text, x, y, maxWidth) {
                        var lines = [];
                        var currentLine = [];
                        var words = text.split(' ');
                        var lineHeight = fontSize * 1.2;
                        var currentY = y;

                        words.forEach(word => {
                            var testLine = currentLine.concat(word).join(' ');
                            var metrics = context.measureText(testLine);

                            if (metrics.width <= maxWidth) {
                                currentLine.push(word);
                            } else {
                                // Try word splitting if line is empty
                                if (currentLine.length === 0) {
                                    var splitPoint = 0;
                                    for (var i = 1; i <= word.length; i++) {
                                        if (context.measureText(word.substr(0, i)).width <=
                                            maxWidth) {
                                            splitPoint = i;
                                        }
                                    }
                                    if (splitPoint > 0) {
                                        currentLine.push(word.substr(0, splitPoint));
                                        words.unshift(word.substr(splitPoint));
                                    }
                                }

                                lines.push(currentLine.join(' '));
                                currentLine = [word];
                            }
                        });

                        lines.push(currentLine.join(' '));
                        lines.forEach(line => {
                            context.fillText(line, x, currentY);
                            currentY += lineHeight;
                        });
                    }
                    if (maxWidth > 0) {
                        pdTextWrap(ctx, text, x, y, maxWidth);
                    } else {
                        ctx.fillText(text, x, y);
                    }
                    // clang-format on
                },
                v.current_color, v.layer_id, v.text, v.x, v.y, v.w, v.font_height);
            break;
        case DRAW_SVG:
            // Código para desenhar SVG (ainda a ser implementado)
            break;
        case STROKE_PATH:
            for (int i = 0; i < v.path_size; i++) {
                int x = v.path_coords[i * 2];
                int y = v.path_coords[i * 2 + 1];
                if (i == 0) {
                    EM_ASM(
                        {
                            var layer_id = UTF8ToString($0);
                            var x = $1;
                            var y = $2;
                            var item = document.getElementById(layer_id);
                            if (item == null) {
                                console.log("stroke_path: item not found");
                                return;
                            }
                            const ctx = item.getContext('2d');
                            ctx.beginPath();
                            ctx.moveTo(x, y);
                        },
                        v.layer_id, x, y);
                } else {
                    EM_ASM(
                        {
                            var layer_id = UTF8ToString($0);
                            var x = $1;
                            var y = $2;
                            var item = document.getElementById(layer_id);
                            if (item == null) {
                                console.log("stroke_path: item not found");
                                return;
                            }
                            const ctx = item.getContext('2d');
                            ctx.lineTo(x, y); // Draw line to next point
                        },
                        v.layer_id, x, y);
                }
            }
            EM_ASM(
                {
                    var layer_id = UTF8ToString($0);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("stroke_path: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.strokeStyle = UTF8ToString($1);
                    ctx.lineWidth = $2;
                    ctx.stroke();
                    ctx.closePath();
                },
                v.layer_id, v.current_color, v.stroke_width);
            // freebytes(v.path_coords, v.path_size * sizeof(float));
            break;
        case FILL_PATH:
            for (int i = 0; i < v.path_size; i++) {
                int x = v.path_coords[i * 2];
                int y = v.path_coords[i * 2 + 1];
                if (i == 0) {
                    EM_ASM(
                        {
                            var layer_id = UTF8ToString($0);
                            var x = $1;
                            var y = $2;
                            var item = document.getElementById(layer_id);
                            if (item == null) {
                                console.log("stroke_path: item not found");
                                return;
                            }
                            const ctx = item.getContext('2d');
                            ctx.beginPath();
                            ctx.moveTo(x, y);
                        },
                        v.layer_id, x, y);
                } else {
                    EM_ASM(
                        {
                            var layer_id = UTF8ToString($0);
                            var x = $1;
                            var y = $2;
                            var item = document.getElementById(layer_id);
                            if (item == null) {
                                console.log("stroke_path: item not found");
                                return;
                            }
                            const ctx = item.getContext('2d');
                            ctx.lineTo(x, y); // Draw line to next point
                        },
                        v.layer_id, x, y);
                }
            }
            EM_ASM(
                {
                    var layer_id = UTF8ToString($0);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("stroke_path: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.fillStyle = UTF8ToString($1);
                    ctx.fill();
                    ctx.closePath();
                },
                v.layer_id, v.current_color);
            // freebytes(v.path_coords, v.path_size * sizeof(float));
            break;
        case FILL_ALL:
            MAIN_THREAD_ASYNC_EM_ASM(
                {
                    var color = UTF8ToString($0);
                    var layer_id = UTF8ToString($1);
                    var item = document.getElementById(layer_id);
                    if (item == null) {
                        console.log("fill_all: item not found");
                        return;
                    }
                    const ctx = item.getContext('2d');
                    ctx.fillStyle = color;
                    ctx.fillRect(1, 1, item.width - 2, item.height - 2);
                },
                v.current_color, v.layer_id);
            break;
        default:
            // Código para comando desconhecido
            break;
        }
        LuaGuiQueue->values[i].drawed = 1;
    }
    return 0;
}

// ─────────────────────────────────────
static int pd4web_get_next_free_spot() {
    int old_size = LuaGuiQueue->size;
    for (int i = LuaGuiQueue->i; i < LuaGuiQueue->size; i++) {
        if (LuaGuiQueue->values[i].drawed) {
            LuaGuiQueue->values[i].drawed = 0;
            LuaGuiQueue->i = (i + 1) % LuaGuiQueue->size;
            return i;
        }
    }

    int new_size = LuaGuiQueue->size * 2;
    GuiValues *new_values = (GuiValues *)realloc(LuaGuiQueue->values, sizeof(GuiValues) * new_size);
    if (new_values == NULL) {
        MAIN_THREAD_ASYNC_EM_ASM({ alert("Failed to malloc memory"); });
        return 0;
    }
    LuaGuiQueue->values = new_values;
    LuaGuiQueue->size = new_size;
    printf("bad Found free spot at %d\n", old_size + 1);
    return old_size + 1;
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

    int x = text_xpix((t_object *)o, o->canvas);
    int y = text_ypix((t_object *)o, o->canvas);
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
                                       {"draw_svg", draw_svg}, // TODO:
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
    printf("mallocing memory\n");
    if (LuaGuiQueue == NULL) {
        LuaGuiQueue = (Pd4WebLuaGuiQueue *)malloc(sizeof(Pd4WebLuaGuiQueue));
        if (LuaGuiQueue == NULL) {
            MAIN_THREAD_ASYNC_EM_ASM({ alert("Failed to malloc memory"); });
            return 0;
        }
        LuaGuiQueue->values = (GuiValues *)malloc(sizeof(GuiValues) * 1024);
        if (LuaGuiQueue->values == NULL) {
            MAIN_THREAD_ASYNC_EM_ASM({ alert("Failed to malloc memory"); });
            free(LuaGuiQueue); // Liberar a memória previamente alocada
            return 0;
        }
        LuaGuiQueue->size = 1024;
        LuaGuiQueue->i = 0;
        for (int i = 0; i < LuaGuiQueue->size; i++) {
            LuaGuiQueue->values[i].drawed = 1;
        }
    }

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
    printf("start_paint\n");

    t_pdlua *obj = (t_pdlua *)lua_touserdata(L, 1);
    t_pdlua_gfx *gfx = &obj->gfx;

    if (gfx->object_tag[0] == '\0') {
        lua_pushnil(L);
        return 1;
    }

    int layer = luaL_checknumber(L, 2) - 1;
    if (layer > gfx->num_layers) {
        GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
        strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
        strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
        v->command = CLEAR_CANVAS;
        v->drawed = 0;
        pdlua_gfx_repaint(obj, 0);
        lua_pushnil(L);
        return 1;
    } else if (layer >= gfx->num_layers) {
        int new_num_layers = layer + 1;
        if (gfx->layer_tags) {
            gfx->layer_tags = resizebytes(gfx->layer_tags, sizeof(char *) * gfx->num_layers,
                                          sizeof(char *) * new_num_layers);
        } else {
            gfx->layer_tags = getbytes(sizeof(char *));
        }

        gfx->layer_tags[layer] = getbytes(64);
        snprintf(gfx->layer_tags[layer], 64, "%p_%d", obj, layer);
        gfx->num_layers = new_num_layers;
        int x = text_xpix((t_object *)obj, obj->canvas);
        int y = text_ypix((t_object *)obj, obj->canvas);

        // Maybe this is better to live also on main thread
        MAIN_THREAD_ASYNC_EM_ASM(
            {
                let layer_id = UTF8ToString($0);
                let x_pos = $1;
                let y_pos = $2;
                let width = $3;
                let height = $4;
                let zoom = $5;

                const container = document.getElementById("Pd4WebPatchDiv");
                var item = document.getElementById(layer_id);
                if (document.getElementById(layer_id) == null) {
                    item = document.createElement("canvas");
                    container.appendChild(item);
                } else {
                    item = document.getElementById(layer_id);
                }

                const containerX = container.getBoundingClientRect().left;
                const containerY = container.getBoundingClientRect().top;

                item.id = layer_id;
                item.width = width * zoom;
                item.height = height * zoom;
                item.style.position = "absolute";
                item.style.left = containerX + (x_pos * zoom) + "px";
                item.style.top = containerY + (y_pos * zoom) + "px";

                const ctx = item.getContext('2d');
                ctx.strokeRect(0, 0, width * zoom, height * zoom);
            },
            gfx->layer_tags[layer], x, y, gfx->width, gfx->height, PD4WEB_PATCH_ZOOM);
    }

    gfx->current_layer_tag = gfx->layer_tags[layer];
    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = CLEAR_CANVAS;
    v->drawed = 0;

    if (gfx->transforms) {
        freebytes(gfx->transforms, gfx->num_transforms * sizeof(gfx_transform));
    }
    gfx->num_transforms = 0;
    gfx->transforms = NULL;

    lua_pushlightuserdata(L, gfx);
    luaL_setmetatable(L, "GraphicsContext");

    if (strlen(gfx->object_tag)) {
        pdlua_gfx_clear(obj, layer, 0);
    }

    if (gfx->first_draw) {
        int x = text_xpix((t_object *)obj, obj->canvas);
        int y = text_ypix((t_object *)obj, obj->canvas);
    }
    return 1;
}

// ─────────────────────────────────────
static int end_paint(lua_State *L) {
    // TODO: What to do here?
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = (t_pdlua *)gfx->object;
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    int scale = glist_getzoom(glist_getcanvas(obj->canvas));
    int layer = luaL_checknumber(L, 1) - 1;

    // Draw iolets on top
    int xpos = text_xpix((t_object *)obj, obj->canvas);
    int ypos = text_ypix((t_object *)obj, obj->canvas);

    // TODO: I don't think we need to call drawiofor on each layer?
    glist_drawiofor(obj->canvas, (t_object *)obj, 1, gfx->object_tag, xpos, ypos,
                    xpos + (gfx->width * scale), ypos + (gfx->height * scale));

    if (!gfx->first_draw && gfx->order_tag[0] != '\0') {

        // Move everything to below the order marker, to make sure redrawn stuff isn't always on
        // top
        pdgui_vmess(0, "crss", cnv, "lower", gfx->object_tag, gfx->order_tag);

        if (layer == 0 && gfx->num_layers > 1) {
            if (layer < gfx->num_layers)
                pdgui_vmess(0, "crss", cnv, "lower", gfx->current_layer_tag,
                            gfx->layer_tags[layer + 1]);
        } else if (layer != 0) {
            pdgui_vmess(0, "crss", cnv, "raise", gfx->current_layer_tag,
                        gfx->layer_tags[layer - 1]);
        }
    }

    return 0;
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

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = FILL_ELLIPSE;
    v->x = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->h = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->drawed = 0;
    return 0;
}

// ─────────────────────────────────────
static int stroke_ellipse(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = STROKE_ELLIPSE;
    v->x = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->h = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->line_width = luaL_checknumber(L, 5) * PD4WEB_PATCH_ZOOM;
    v->drawed = 0;
    return 0;
}

// ─────────────────────────────────────
static int fill_all(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = FILL_ALL;
    v->drawed = 0;

    return 0;
}

// ─────────────────────────────────────
static int fill_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = FILL_RECT;
    v->x = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->h = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->drawed = 0;

    return 0;
}

// ─────────────────────────────────────
static int stroke_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = STROKE_RECT;
    v->x = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->h = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->line_width = luaL_checknumber(L, 5) * PD4WEB_PATCH_ZOOM;
    LuaGuiQueue->i += 1;

    return 0;
}

// ─────────────────────────────────────
static int fill_rounded_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = FILL_ROUNDED_RECT;
    v->x = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->h = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->radius = luaL_checknumber(L, 5) * PD4WEB_PATCH_ZOOM;
    LuaGuiQueue->i += 1;
    return 0;
}
// ─────────────────────────────────────
static int stroke_rounded_rect(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = STROKE_ROUNDED_RECT;
    v->x = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->h = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->radius = luaL_checknumber(L, 5) * PD4WEB_PATCH_ZOOM;
    v->drawed = 0;

    return 0;
}

// ─────────────────────────────────────
static int draw_line(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = DRAW_LINE;
    v->x1 = luaL_checknumber(L, 1) * PD4WEB_PATCH_ZOOM;
    v->y1 = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->x2 = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->y2 = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->line_width = luaL_checknumber(L, 5) * PD4WEB_PATCH_ZOOM;
    v->drawed = 0;

    return 0;
}

// ─────────────────────────────────────
static int draw_text(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = DRAW_TEXT;
    v->text = luaL_checkstring(L, 1); // TODO: Fix this
    v->x = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    v->y = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;
    v->w = luaL_checknumber(L, 4) * PD4WEB_PATCH_ZOOM;
    v->font_height = luaL_checknumber(L, 5) * PD4WEB_PATCH_ZOOM;
    v->drawed = 0;

    return 0;
}
// clang-format on

// ─────────────────────────────────────
static uint64_t pdlua_image_hash(unsigned char *str) {
    uint64_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

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
    if (encoded_data == NULL)
        return NULL;

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

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    encoded_data[output_length] = '\0';
    return encoded_data;
}

// ─────────────────────────────────────
static int draw_svg(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    int canvas_zoom = PD4WEB_PATCH_ZOOM;

    float scale_x = canvas_zoom, scale_y = canvas_zoom;
    transform_size_float(gfx, &scale_x, &scale_y);
    float scale = (scale_x + scale_y) * 0.5f;

    char *svg_text = strdup(luaL_checkstring(L, 1));
    int x = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    int y = luaL_checknumber(L, 3) * PD4WEB_PATCH_ZOOM;

    MAIN_THREAD_ASYNC_EM_ASM(
        {
            var color = UTF8ToString($0);
            var objpointer = UTF8ToString($1);
            var svgText = UTF8ToString($2);
            var x = $3;
            var y = $4;
            var scale = $5;

            var canvas = document.getElementById(objpointer);
            if (!canvas) {
                console.error("draw_svg: Canvas not found");
                return;
            }

            var ctx = canvas.getContext('2d');

            // Create Blob from SVG text
            var svgBlob = new Blob([svgText], {
                type:
                    'image/svg+xml;charset=utf-8'
            });
            var url = URL.createObjectURL(svgBlob);

            var img = new Image();
            img.onload = function() {
                // Save context state
                ctx.save();

                // Apply Pd-style transformations
                ctx.translate(x, y);
                ctx.scale(scale, scale);

                // Apply color tint (matches Pd's fill color behavior)
                ctx.globalCompositeOperation = 'source-atop';
                ctx.drawImage(img, 0, 0);

                // Apply color overlay
                ctx.fillStyle = color;
                ctx.globalAlpha = 0.99; // Force composite
                ctx.fillRect(0, 0, img.width, img.height);

                ctx.restore();

                // Cleanup
                URL.revokeObjectURL(url);
            };
            img.onerror = function() {
                console.error("Error loading SVG image");
                URL.revokeObjectURL(url);
            };
            img.src = url;
        },
        gfx->current_color, gfx->current_layer_tag, svg_text, x, y, scale);

    free(svg_text);
    return 0;
}

// ─────────────────────────────────────
static int stroke_path(lua_State *L) {
    // Retrieve graphics context
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_pdlua *obj = gfx->object;
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    // Get the path and check if it has enough segments
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (path->num_path_segments < 3) {
        return 0;
    }
    int stroke_width = luaL_checknumber(L, 2) * PD4WEB_PATCH_ZOOM;
    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = STROKE_PATH;
    // free after complete draw in main thread
    v->path_coords = getbytes(path->num_path_segments * 2 * sizeof(t_float));
    v->path_size = path->num_path_segments;
    v->line_width = stroke_width;

    for (int i = 0; i < path->num_path_segments; i++) {
        float x = path->path_segments[i * 2], y = path->path_segments[i * 2 + 1];
        transform_point_float(gfx, &x, &y);
        v->path_coords[i * 2] = (x * PD4WEB_PATCH_ZOOM);
        v->path_coords[i * 2 + 1] = (y * PD4WEB_PATCH_ZOOM);
    }
    v->drawed = 0;
    return 0;
}

// ─────────────────────────────────────
static int fill_path(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (path->num_path_segments < 3) {
        return 0;
    }

    GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    v->command = FILL_PATH;
    // free after complete draw in main thread
    v->path_coords = getbytes(path->num_path_segments * 2 * sizeof(t_float));
    v->path_size = path->num_path_segments;

    for (int i = 0; i < path->num_path_segments; i++) {
        float x = path->path_segments[i * 2], y = path->path_segments[i * 2 + 1];
        transform_point_float(gfx, &x, &y);
        v->path_coords[i * 2] = (x * PD4WEB_PATCH_ZOOM);
        v->path_coords[i * 2 + 1] = (y * PD4WEB_PATCH_ZOOM);
    }
    v->drawed = 0;

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
