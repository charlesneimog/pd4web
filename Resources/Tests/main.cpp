#include <algorithm>
#include <format>
#include <iostream>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <unordered_map>

#include <fontconfig/fontconfig.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

#include <cstdio>

// Your Pd4WebUserData struct, simplified for demo
struct Pd4WebUserData {
    GLFWwindow *window = nullptr;
    NVGcontext *vg = nullptr;
    int canvas_width = 800;
    int canvas_height = 600;
    bool contextReady = false;
    int font_handler = -1;
    bool redraw = true;
    float devicePixelRatio = 1.0f;
    NVGLUframebuffer *fb;
    // add other members as needed
};

void getGlCtx(Pd4WebUserData *ud) {
    if (ud->contextReady) {
        glfwMakeContextCurrent(ud->window);
        return;
    }

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    ud->window = glfwCreateWindow(ud->canvas_width, ud->canvas_height, "NanoVG Test", NULL, NULL);
    if (!ud->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(ud->window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW init failed: %s\n", glewGetErrorString(err));
        glfwDestroyWindow(ud->window);
        glfwTerminate();
        return;
    }

    ud->vg = nvgCreateGLES3(0);
    if (!ud->vg) {
        fprintf(stderr, "Failed to create NanoVG context\n");
        glfwDestroyWindow(ud->window);
        glfwTerminate();
        return;
    }

    ud->font_handler = nvgCreateFont(
        ud->vg, "roboto", "/home/neimog/Documents/Git/pd4web/Sources/Pd4Web/DejaVuSans.ttf");
    if (ud->font_handler == -1) {
        printf("Failed to create NanoVG font\n");
        exit(-1);
        return;
    }

    ud->contextReady = true;
}

// ─────────────────────────────────────
void loop(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    if (!ud->redraw) {
        return;
    }

    getGlCtx(ud);

    if (!ud->fb) {
        ud->fb = nvgluCreateFramebuffer(ud->vg, ud->canvas_width, ud->canvas_height,
                                        NVG_IMAGE_PREMULTIPLIED);
        if (!ud->fb) {
            printf("Failed to create framebuffer\n");
            exit(-1);
            return;
        }

        // Check framebuffer completeness right after creation
        glBindFramebuffer(GL_FRAMEBUFFER, ud->fb->fbo);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "Framebuffer not complete: 0x%x\n", status);
            exit(-1);
        }
    }

    const float pixelScale = ud->devicePixelRatio;

    if (!ud->fb) {
        fprintf(stderr, "Framebuffer is null\n");
        exit(-1);
    }

    // Bind framebuffer ONCE before drawing
    nvgluBindFramebuffer(ud->fb);

    nvgViewport(0, 0, ud->canvas_width, ud->canvas_height);
    nvgClear(ud->vg);

    // Enable scissor test and set scissor rectangle explicitly
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 100, 100);

    nvgBeginFrame(ud->vg, ud->canvas_width, ud->canvas_height, pixelScale);
    nvgGlobalScissor(ud->vg, 0, 0, 100, 100);

    nvgBeginPath(ud->vg);
    nvgRect(ud->vg, 0, 0, 50, 50);
    nvgFillColor(ud->vg, nvgRGBA(255, 255, 0, 255));
    nvgFill(ud->vg);

    nvgEndFrame(ud->vg);

    // // Unbind framebuffer: bind default framebuffer for drawing
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    // // Setup correct framebuffer bindings for blit
    // glBindFramebuffer(GL_READ_FRAMEBUFFER, ud->fb->fbo);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // // Set viewport to window size before blitting
    // glViewport(0, 0, ud->canvas_width, ud->canvas_height);

    // // Perform blit - using OpenGL call directly (nvgluBlitFramebuffer might do this internally)
    // glBlitFramebuffer(0, 0, ud->canvas_width, ud->canvas_height, 0, 0, ud->canvas_width,
    //                   ud->canvas_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    //
    // // Unbind read framebuffer
    // glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    ud->redraw = false;

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("GL error: %d\n", err);
    }
}

// ─────────────────────────────────────
int main() {
    Pd4WebUserData ud;

    FcInit();
    getGlCtx(&ud);
    if (!ud.contextReady) {
        return -1;
    }

    while (!glfwWindowShouldClose(ud.window)) {
        ud.redraw = true;
        loop(&ud);

        glfwSwapBuffers(ud.window);
        glfwPollEvents();
    }

    if (ud.vg) {
        nvgDeleteGLES3(ud.vg);
    }
    if (ud.window) {
        glfwDestroyWindow(ud.window);
    }
    glfwTerminate();

    return 0;
}
