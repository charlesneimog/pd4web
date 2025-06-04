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
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/threading.h>

#include <GLES3/gl3.h>

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
    float font_height;
    float stroke_width;

    float canvas_width;
    float canvas_height;

    float *path_coords;
    int path_size;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    GLuint Shader;
} GuiValues;

#define PDLUA_MAX_COMMANDS 128
static GuiValues commandQueue[PDLUA_MAX_COMMANDS];
static int queueStart = 0;
static int queueEnd = 0;

// ╭─────────────────────────────────────╮
// │               SHADERS               │
// ╰─────────────────────────────────────╯
typedef struct {
    bool compiled;
    GLuint fill_rect;
} ShadersProgram;

ShadersProgram Pd4WebShaders;

// ──────────────────────────────────────────
GLuint compile_shader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    return shader;
}

// ──────────────────────────────────────────
static GLuint shader_compile(const char *vertex_src, const char *fragment_src) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_src);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        emscripten_log(EM_LOG_ERROR, "Program linking failed: %s", infoLog);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

// ──────────────────────────────────────────
static void compile_all_shaders() {
    const char *vertex_src = "#version 300 es\n"
                             "layout(location = 0) in vec2 aPos;\n"
                             "void main() {\n"
                             "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
                             "}\n";

    const char *fragment_src = "#version 300 es\n"
                               "precision mediump float;\n"
                               "uniform vec3 uColor;\n"
                               "out vec4 FragColor;\n"
                               "void main() {\n"
                               "    FragColor = vec4(uColor, 1.0);\n"
                               "}\n";

    Pd4WebShaders.fill_rect = shader_compile(vertex_src, fragment_src);
}
// ╭─────────────────────────────────────╮
// │  This will be called from the main  │
// │   thread inside pd4web main loop    │
// ╰─────────────────────────────────────╯
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE create_webgl_context_for_layer(char *tag) {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = true;
    attr.depth = false;
    attr.stencil = false;
    attr.antialias = true;
    attr.premultipliedAlpha = false;
    attr.preserveDrawingBuffer = false;
    attr.failIfMajorPerformanceCaveat = false;

    attr.majorVersion = 2;
    attr.minorVersion = 0;

    attr.enableExtensionsByDefault = false;
    attr.explicitSwapControl = false;
    attr.renderViaOffscreenBackBuffer = false;

    char selector[128];
    snprintf(selector, 128, "#%s", tag);

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(selector, &attr);
    if (ctx <= 0) {
        printf("Failed to create WebGL context for canvas: %s (error: %l)\n", selector, ctx);
        return 0;
    }

    if (emscripten_webgl_make_context_current(ctx) != EMSCRIPTEN_RESULT_SUCCESS) {
        emscripten_webgl_destroy_context(ctx);
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

// ─────────────────────────────────────
void add_quad(float *vertices, int *offset, float x0, float y0, float x1, float y1, float x2,
              float y2, float x3, float y3) {
    // First triangle
    vertices[(*offset)++] = x0;
    vertices[(*offset)++] = y0;
    vertices[(*offset)++] = x1;
    vertices[(*offset)++] = y1;
    vertices[(*offset)++] = x2;
    vertices[(*offset)++] = y2;
    // Second triangle
    vertices[(*offset)++] = x0;
    vertices[(*offset)++] = y0;
    vertices[(*offset)++] = x2;
    vertices[(*offset)++] = y2;
    vertices[(*offset)++] = x3;
    vertices[(*offset)++] = y3;
}

// ─────────────────────────────────────
void add_arc_strip(float *vertices, int *offset, float cx, float cy, float inner_radius,
                   float outer_radius, float start_angle, float end_angle, int num_segments) {
    float da = (end_angle - start_angle) / num_segments;
    for (int i = 0; i < num_segments; ++i) {
        float a0 = start_angle + i * da;
        float a1 = start_angle + (i + 1) * da;
        // Outer arc
        float x0 = cx + cosf(a0) * outer_radius;
        float y0 = cy + sinf(a0) * outer_radius;
        float x1 = cx + cosf(a1) * outer_radius;
        float y1 = cy + sinf(a1) * outer_radius;
        // Inner arc
        float ix0 = cx + cosf(a0) * inner_radius;
        float iy0 = cy + sinf(a0) * inner_radius;
        float ix1 = cx + cosf(a1) * inner_radius;
        float iy1 = cy + sinf(a1) * inner_radius;

        // First triangle
        vertices[(*offset)++] = x0;
        vertices[(*offset)++] = y0;
        vertices[(*offset)++] = x1;
        vertices[(*offset)++] = y1;
        vertices[(*offset)++] = ix1;
        vertices[(*offset)++] = iy1;
        // Second triangle
        vertices[(*offset)++] = x0;
        vertices[(*offset)++] = y0;
        vertices[(*offset)++] = ix1;
        vertices[(*offset)++] = iy1;
        vertices[(*offset)++] = ix0;
        vertices[(*offset)++] = iy0;
    }
}

#define NUM_ARC_SEGMENTS 12

// ─────────────────────────────────────
int pd4weblua_draw() {
    GuiValues cmd;
    while (dequeue_command(&cmd)) {
        switch (cmd.command) {
        case FILL_ALL: {
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);
            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                glClearColor(r, g, b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            break;
        }
        case FILL_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                // 1. Configurar shaders (apenas na primeira execução)
                GLuint program = 0;
                GLuint vertex_buffer = 0;

                // Vertex Shader
                const char *vertex_shader_src = "attribute vec2 a_position;"
                                                "void main() {"
                                                "   gl_Position = vec4(a_position, 0.0, 1.0);"
                                                "}";

                // Fragment Shader
                const char *fragment_shader_src = "precision mediump float;"
                                                  "uniform vec4 u_color;"
                                                  "void main() {"
                                                  "   gl_FragColor = u_color;"
                                                  "}";

                // Compilar shaders
                GLuint vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vertex_shader_src, NULL);
                glCompileShader(vs);

                GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fragment_shader_src, NULL);
                glCompileShader(fs);

                // Linkar programa
                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);
                glUseProgram(program);

                // Criar buffer
                glGenBuffers(1, &vertex_buffer);

                // 2. Calcular coordenadas normalizadas (-1 a 1)
                float canvas_width = cmd.canvas_width;
                float canvas_height = cmd.canvas_height;

                float x0 = 2.0f * (x / canvas_width) - 1.0f;
                float x1 = 2.0f * ((x + width) / canvas_width) - 1.0f;
                float y0 = 1.0f - 2.0f * ((y + height) / canvas_height); // Top
                float y1 = 1.0f - 2.0f * (y / canvas_height);            // Bottom

                // 3. Definir vértices do retângulo (dois triângulos)
                float vertices[] = {x0, y0,                 // Triângulo 1
                                    x1, y0, x0, y1, x0, y1, // Triângulo 2
                                    x1, y0, x1, y1};

                // 4. Passar vértices para o buffer
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                // 5. Configurar atributo de vértice
                GLuint a_position = glGetAttribLocation(program, "a_position");
                glEnableVertexAttribArray(a_position);
                glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

                // 6. Passar cor uniforme
                GLuint u_color = glGetUniformLocation(program, "u_color");
                glUniform4f(u_color, r, g, b, 1.0f);

                // 7. Desenhar
                glDrawArrays(GL_TRIANGLES, 0, 6);
                break;
            }
        }
        case STROKE_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float thickness = cmd.line_width; // New: desired stroke width
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                GLuint program;
                GLuint vertex_buffer;

                const char *vertex_shader_src = "attribute vec2 a_position;"
                                                "void main() {"
                                                "   gl_Position = vec4(a_position, 0.0, 1.0);"
                                                "}";

                const char *fragment_shader_src = "precision mediump float;"
                                                  "uniform vec4 u_color;"
                                                  "void main() {"
                                                  "   gl_FragColor = u_color;"
                                                  "}";

                GLuint vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vertex_shader_src, NULL);
                glCompileShader(vs);

                GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fragment_shader_src, NULL);
                glCompileShader(fs);

                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);
                glUseProgram(program);

                glGenBuffers(1, &vertex_buffer);

                glDisable(GL_BLEND);

                float canvas_width = cmd.canvas_width;
                float canvas_height = cmd.canvas_height;

                glViewport(0, 0, canvas_width * PD4WEB_PATCH_ZOOM,
                           canvas_height * PD4WEB_PATCH_ZOOM);

                // Convert rectangle corners to NDC
                float x0 = 2.0f * (x / canvas_width) - 1.0f;
                float x1 = 2.0f * ((x + width) / canvas_width) - 1.0f;
                float y0 = 1.0f - 2.0f * ((y + height) / canvas_height);
                float y1 = 1.0f - 2.0f * (y / canvas_height);

                // Thickness in NDC
                float t_x = 2.0f * (thickness / canvas_width);
                float t_y = 2.0f * (thickness / canvas_height);

                // Vertex array for four quads (4 edges, 6 vertices per quad, 2 coords per vert)
                float vertices[4 * 6 * 2];
                int offset = 0;

                // Top edge
                add_quad(vertices, &offset, x0, y1, x1, y1, x1, y1 - t_y, x0, y1 - t_y);

                // Bottom edge
                add_quad(vertices, &offset, x0, y0 + t_y, x1, y0 + t_y, x1, y0, x0, y0);

                // Left edge
                add_quad(vertices, &offset, x0, y1 - t_y, x0 + t_x, y1 - t_y, x0 + t_x, y0 + t_y,
                         x0, y0 + t_y);

                // Right edge
                add_quad(vertices, &offset, x1 - t_x, y1 - t_y, x1, y1 - t_y, x1, y0 + t_y,
                         x1 - t_x, y0 + t_y);

                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                GLuint a_position = glGetAttribLocation(program, "a_position");
                glEnableVertexAttribArray(a_position);
                glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

                GLuint u_color = glGetUniformLocation(program, "u_color");
                glUniform4f(u_color, r, g, b, 1.0f);

                glDrawArrays(GL_TRIANGLES, 0, 4 * 6);

                glDeleteProgram(program);
                glDeleteShader(vs);
                glDeleteShader(fs);
                glDeleteBuffers(1, &vertex_buffer);
            }
            break;
        }
        case FILL_ELLIPSE: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                // Recompilar shaders sempre
                GLuint program;
                GLuint vertex_buffer;

                // Vertex Shader
                const char *vertex_shader_src = "attribute vec2 a_position;"
                                                "void main() {"
                                                "   gl_Position = vec4(a_position, 0.0, 1.0);"
                                                "}";

                // Fragment Shader
                const char *fragment_shader_src = "precision mediump float;"
                                                  "uniform vec4 u_color;"
                                                  "void main() {"
                                                  "   gl_FragColor = u_color;"
                                                  "}";

                GLuint vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vertex_shader_src, NULL);
                glCompileShader(vs);

                GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fragment_shader_src, NULL);
                glCompileShader(fs);

                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);
                glUseProgram(program);

                glGenBuffers(1, &vertex_buffer);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                double dpr = emscripten_get_device_pixel_ratio();
                float canvas_width = cmd.canvas_width;
                float canvas_height = cmd.canvas_height;
                glViewport(0, 0, canvas_width * PD4WEB_PATCH_ZOOM,
                           canvas_height * PD4WEB_PATCH_ZOOM);

                float cx = x + width / 2.0f;
                float cy = y + height / 2.0f;
                float rx = width / 2.0f;
                float ry = height / 2.0f;

                const int num_segments = 128;
                float vertices[(num_segments + 2) * 2]; // center + segments + wrap

                float norm_cx = 2.0f * (cx / canvas_width) - 1.0f;
                float norm_cy = 1.0f - 2.0f * (cy / canvas_height);
                vertices[0] = norm_cx;
                vertices[1] = norm_cy;

                for (int i = 0; i <= num_segments; ++i) {
                    float angle = (2.0f * M_PI * i) / num_segments;
                    float dx = rx * cosf(angle);
                    float dy = ry * sinf(angle);

                    float vx = 2.0f * ((cx + dx) / canvas_width) - 1.0f;
                    float vy = 1.0f - 2.0f * ((cy + dy) / canvas_height);

                    vertices[(i + 1) * 2 + 0] = vx;
                    vertices[(i + 1) * 2 + 1] = vy;
                }

                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                GLuint a_position = glGetAttribLocation(program, "a_position");
                glEnableVertexAttribArray(a_position);
                glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

                GLuint u_color = glGetUniformLocation(program, "u_color");
                glUniform4f(u_color, r, g, b, 1.0f);

                glDrawArrays(GL_TRIANGLE_FAN, 0, num_segments + 2);

                // Limpeza (opcional, mas recomendável nesse modo)
                glDeleteProgram(program);
                glDeleteShader(vs);
                glDeleteShader(fs);
                glDeleteBuffers(1, &vertex_buffer);
            }
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

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                GLuint program;
                GLuint vertex_buffer;

                // Vertex Shader
                const char *vertex_shader_src = "attribute vec2 a_position;"
                                                "void main() {"
                                                "  gl_Position = vec4(a_position, 0.0, 1.0);"
                                                "}";

                // Fragment Shader
                const char *fragment_shader_src = "precision mediump float;"
                                                  "uniform vec4 u_color;"
                                                  "void main() {"
                                                  "  gl_FragColor = u_color;"
                                                  "}";

                GLuint vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vertex_shader_src, NULL);
                glCompileShader(vs);

                GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fragment_shader_src, NULL);
                glCompileShader(fs);

                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);
                glUseProgram(program);

                glGenBuffers(1, &vertex_buffer);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                double dpr = emscripten_get_device_pixel_ratio();
                float canvas_width = cmd.canvas_width;
                float canvas_height = cmd.canvas_height;
                glViewport(0, 0, canvas_width * PD4WEB_PATCH_ZOOM,
                           canvas_height * PD4WEB_PATCH_ZOOM);

                float cx = x + width / 2.0f;
                float cy = y + height / 2.0f;
                float rx_outer = width / 2.0f;
                float ry_outer = height / 2.0f;
                float rx_inner = rx_outer - line_width;
                float ry_inner = ry_outer - line_width;

                if (rx_inner < 0)
                    rx_inner = 0;
                if (ry_inner < 0)
                    ry_inner = 0;

                const int num_segments = 128;
                // Cada segmento gera dois vértices: externo e interno (strip)
                float vertices[(num_segments + 1) * 4]; // (num_segments+1)*2 pontos * 2 coords

                for (int i = 0; i <= num_segments; ++i) {
                    float angle = (2.0f * M_PI * i) / num_segments;
                    float cos_a = cosf(angle);
                    float sin_a = sinf(angle);

                    // Outer vertex
                    float vx_outer = 2.0f * ((cx + rx_outer * cos_a) / canvas_width) - 1.0f;
                    float vy_outer = 1.0f - 2.0f * ((cy + ry_outer * sin_a) / canvas_height);

                    // Inner vertex
                    float vx_inner = 2.0f * ((cx + rx_inner * cos_a) / canvas_width) - 1.0f;
                    float vy_inner = 1.0f - 2.0f * ((cy + ry_inner * sin_a) / canvas_height);

                    vertices[i * 4 + 0] = vx_outer;
                    vertices[i * 4 + 1] = vy_outer;
                    vertices[i * 4 + 2] = vx_inner;
                    vertices[i * 4 + 3] = vy_inner;
                }

                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                GLuint a_position = glGetAttribLocation(program, "a_position");
                glEnableVertexAttribArray(a_position);
                glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

                GLuint u_color = glGetUniformLocation(program, "u_color");
                glUniform4f(u_color, r, g, b, 1.0f);

                // Usar TRIANGLE_STRIP para conectar os pares de vértices interno e externo
                glDrawArrays(GL_TRIANGLE_STRIP, 0, (num_segments + 1) * 2);

                glDeleteProgram(program);
                glDeleteShader(vs);
                glDeleteShader(fs);
                glDeleteBuffers(1, &vertex_buffer);
            }
            break;
        }

        case STROKE_ROUNDED_RECT: {
            float x = cmd.x1;
            float y = cmd.y1;
            float width = cmd.width;
            float height = cmd.height;
            float radius = cmd.radius;
            float line_width = cmd.line_width;
            float r, g, b;
            hex_to_rgb_normalized(cmd.current_color, &r, &g, &b);

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                GLuint program;
                GLuint vertex_buffer;

                // Vertex shader
                const char *vertex_shader_src = "attribute vec2 a_position;\n"
                                                "varying vec2 v_pos;\n"
                                                "void main() {\n"
                                                "  v_pos = a_position;\n"
                                                "  gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                                "}";

                // Fragment shader
                const char *fragment_shader_src =
                    "precision mediump float;\n"
                    "uniform vec4 u_color;\n"
                    "uniform vec4 u_rect;\n"
                    "uniform float u_radius;\n"
                    "uniform float u_line_width;\n"
                    "varying vec2 v_pos;\n"
                    "\n"
                    "float udRoundBox(vec2 p, vec2 b, float r) {\n"
                    "  return length(max(abs(p) - b, 0.0)) - r;\n"
                    "}\n"
                    "\n"
                    "float opS(float d1, float d2) {\n"
                    "  return max(-d1, d2);\n"
                    "}\n"
                    "\n"
                    "float udRoundBoxBorder(vec2 p, vec2 b, float r, float line_width) {\n"
                    "  return opS(udRoundBox(p, b - vec2(line_width), r - line_width), \n"
                    "             udRoundBox(p, b, r));\n"
                    "}\n"
                    "\n"
                    "void main() {\n"
                    "  vec2 uv = v_pos * 0.5 + 0.5;\n"
                    "  vec2 center = u_rect.xy + u_rect.zw * 0.5;\n"
                    "  vec2 p = uv - center;\n"
                    "  vec2 half_size = u_rect.zw * 0.5;\n"
                    "\n"
                    "  float dist = udRoundBoxBorder(p, half_size, u_radius, u_line_width);\n"
                    "  if (dist > 0.0) discard;\n"
                    "\n"
                    "  gl_FragColor = u_color;\n"
                    "}";

                // Compile vertex shader
                GLuint vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vertex_shader_src, NULL);
                glCompileShader(vs);

                GLint compiled;
                glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
                if (!compiled) {
                    char info[512];
                    glGetShaderInfoLog(vs, 512, NULL, info);
                    printf("Vertex shader compile error: %s\n", info);
                    glDeleteShader(vs);
                    break;
                }

                // Compile fragment shader
                GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fragment_shader_src, NULL);
                glCompileShader(fs);

                glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
                if (!compiled) {
                    char info[512];
                    glGetShaderInfoLog(fs, 512, NULL, info);
                    printf("Fragment shader compile error: %s\n", info);
                    glDeleteShader(fs);
                    glDeleteShader(vs);
                    break;
                }

                // Create program and attach shaders
                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);

                GLint linked;
                glGetProgramiv(program, GL_LINK_STATUS, &linked);
                if (!linked) {
                    char info[512];
                    glGetProgramInfoLog(program, 512, NULL, info);
                    printf("Program link error: %s\n", info);
                    glDeleteProgram(program);
                    glDeleteShader(vs);
                    glDeleteShader(fs);
                    break;
                }

                glUseProgram(program);
                glGenBuffers(1, &vertex_buffer);

                float canvas_width = cmd.canvas_width;
                float canvas_height = cmd.canvas_height;
                glViewport(0, 0, canvas_width * PD4WEB_PATCH_ZOOM,
                           canvas_height * PD4WEB_PATCH_ZOOM);

                // Full screen quad
                float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                GLuint a_position = glGetAttribLocation(program, "a_position");
                if (a_position == -1) {
                    printf("Error: attribute 'a_position' not found.\n");
                    glDeleteProgram(program);
                    glDeleteShader(vs);
                    glDeleteShader(fs);
                    glDeleteBuffers(1, &vertex_buffer);
                    break;
                }
                glEnableVertexAttribArray(a_position);
                glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

                GLuint u_color = glGetUniformLocation(program, "u_color");
                GLuint u_rect = glGetUniformLocation(program, "u_rect");
                GLuint u_radius = glGetUniformLocation(program, "u_radius");
                GLuint u_line_width = glGetUniformLocation(program, "u_line_width");

                glUniform4f(u_color, r, g, b, 1.0f);
                glUniform4f(u_rect, x / canvas_width, y / canvas_height, width / canvas_width,
                            height / canvas_height);
                glUniform1f(u_radius, radius);
                glUniform1f(u_line_width, line_width / fmin(canvas_width, canvas_height));

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                // Cleanup
                glDeleteProgram(program);
                glDeleteShader(vs);
                glDeleteShader(fs);
                glDeleteBuffers(1, &vertex_buffer);
            }
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

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = create_webgl_context_for_layer(cmd.layer_id);
            if (ctx) {
                GLuint program;
                GLuint vertex_buffer;

                const char *vertex_shader_src = "attribute vec2 a_position;"
                                                "void main() {"
                                                "  gl_Position = vec4(a_position, 0.0, 1.0);"
                                                "}";

                const char *fragment_shader_src = "precision mediump float;"
                                                  "uniform vec4 u_color;"
                                                  "void main() {"
                                                  "  gl_FragColor = u_color;"
                                                  "}";

                GLuint vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vertex_shader_src, NULL);
                glCompileShader(vs);

                GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fragment_shader_src, NULL);
                glCompileShader(fs);

                program = glCreateProgram();
                glAttachShader(program, vs);
                glAttachShader(program, fs);
                glLinkProgram(program);
                glUseProgram(program);

                glGenBuffers(1, &vertex_buffer);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                float canvas_width = cmd.canvas_width;
                float canvas_height = cmd.canvas_height;
                glViewport(0, 0, canvas_width * PD4WEB_PATCH_ZOOM,
                           canvas_height * PD4WEB_PATCH_ZOOM);

                // Calcula a direção perpendicular para a espessura da linha
                float dx = x2 - x1;
                float dy = y2 - y1;
                float len = sqrtf(dx * dx + dy * dy);
                if (len == 0)
                    break; // linha degenerate

                float nx = -dy / len;
                float ny = dx / len;

                float half_width = line_width / 2.0f;

                // Quatro vértices do retângulo da linha
                float verts[8];

                // Ponto 1 - lado esquerdo
                verts[0] = 2.0f * ((x1 + nx * half_width) / canvas_width) - 1.0f;
                verts[1] = 1.0f - 2.0f * ((y1 + ny * half_width) / canvas_height);

                // Ponto 1 - lado direito
                verts[2] = 2.0f * ((x1 - nx * half_width) / canvas_width) - 1.0f;
                verts[3] = 1.0f - 2.0f * ((y1 - ny * half_width) / canvas_height);

                // Ponto 2 - lado esquerdo
                verts[4] = 2.0f * ((x2 + nx * half_width) / canvas_width) - 1.0f;
                verts[5] = 1.0f - 2.0f * ((y2 + ny * half_width) / canvas_height);

                // Ponto 2 - lado direito
                verts[6] = 2.0f * ((x2 - nx * half_width) / canvas_width) - 1.0f;
                verts[7] = 1.0f - 2.0f * ((y2 - ny * half_width) / canvas_height);

                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

                GLuint a_position = glGetAttribLocation(program, "a_position");
                glEnableVertexAttribArray(a_position);
                glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

                GLuint u_color = glGetUniformLocation(program, "u_color");
                glUniform4f(u_color, r, g, b, 1.0f);

                // Desenha retângulo da linha com TRIANGLE_STRIP
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glDeleteProgram(program);
                glDeleteShader(vs);
                glDeleteShader(fs);
                glDeleteBuffers(1, &vertex_buffer);
            }
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

    Pd4WebShaders.compiled = false;

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

    t_pdlua *obj = (t_pdlua *)lua_touserdata(L, 1);
    t_pdlua_gfx *gfx = &obj->gfx;

    if (gfx->object_tag[0] == '\0') {
        lua_pushnil(L);
        return 1;
    }

    int layer = luaL_checkinteger(L, 2) - 1;

    if (layer > gfx->num_layers) {
        pdlua_gfx_repaint(obj, 0);
        lua_pushnil(L);
        return 1;
    }
    int new_num_layers = layer + 1;
    if (gfx->layer_tags) {
        gfx->layer_tags = resizebytes(gfx->layer_tags, sizeof(char *) * gfx->num_layers,
                                      sizeof(char *) * new_num_layers);
    } else {
        gfx->layer_tags = getbytes(sizeof(char *));
    }

    if (gfx->first_draw) {
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

                const container = document.getElementById("Pd4WebPatchDiv");
                var item = document.getElementById(layer_id);
                if (document.getElementById(layer_id) == null) {
                    item = document.createElement("canvas");
                    container.appendChild(item);
                } else {
                    item = document.getElementById(layer_id);
                }

                item.id = layer_id;
                item.width = width * zoom;
                item.height = height * zoom;
                item.style.position = "absolute";
                item.style.border = "1px solid #000";
                item.style.left = (x_pos * zoom) + "px";
                item.style.top = (y_pos * zoom) + "px";
            },
            gfx->layer_tags[layer], x, y, gfx->width, gfx->height, PD4WEB_PATCH_ZOOM);
        gfx->first_draw = 0;

        if (!Pd4WebShaders.compiled) {
            // printf("Compiling Pd4Web Shaders\n");
            // create_webgl_context_for_layer(gfx->layer_tags[layer]);
            // Pd4WebShaders.compiled = true;
            // compile_all_shaders();
        }
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

    return 0;
}

// ─────────────────────────────────────
static int draw_text(lua_State *L) { return 0; }

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
static int draw_svg(lua_State *L) {}

// ─────────────────────────────────────
static int stroke_path(lua_State *L) {}

// ─────────────────────────────────────
static int fill_path(lua_State *L) {
    t_pdlua_gfx *gfx = pop_graphics_context(L);
    t_path_state *path = (t_path_state *)luaL_checkudata(L, 1, "Path");
    if (path->num_path_segments < 3) {
        return 0;
    }

    // GuiValues *v = &LuaGuiQueue->values[pd4web_get_next_free_spot()];
    // strlcpy(v->current_color, gfx->current_color, sizeof(v->current_color));
    // strlcpy(v->layer_id, gfx->current_layer_tag, sizeof(v->layer_id));
    // v->command = FILL_PATH;
    // // free after complete draw in main thread
    // v->path_coords = getbytes(path->num_path_segments * 2 * sizeof(t_float));
    // v->path_size = path->num_path_segments;
    //
    // for (int i = 0; i < path->num_path_segments; i++) {
    //     float x = path->path_segments[i * 2], y = path->path_segments[i * 2 + 1];
    //     transform_point_float(gfx, &x, &y);
    //     v->path_coords[i * 2] = (x * PD4WEB_PATCH_ZOOM);
    //     v->path_coords[i * 2 + 1] = (y * PD4WEB_PATCH_ZOOM);
    // }
    // v->drawed = 0;

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
