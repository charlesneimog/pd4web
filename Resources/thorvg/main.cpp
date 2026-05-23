#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#include <cstdio>
#include <cmath>

#include <thorvg.h>

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext = 0;
tvg::GlCanvas *canvas = nullptr;
float zoom = 1.0f;
uint32_t fbWidth = 0;
uint32_t fbHeight = 0;
double dpr = 1.0;

// ─────────────────────────────────────
static void redraw() {
    if (!canvas) {
        return;
    }

    emscripten_webgl_make_context_current(glContext);
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Remove previous paints
    auto paints = canvas->paints();
    for (auto paint : paints) {
        canvas->remove(paint);
    }

    float scale = zoom * static_cast<float>(dpr);

    auto text1 = tvg::Text::gen();
    text1->font("dejavu-sans-webfont");
    text1->text("Hello World");
    text1->size(80);
    text1->fill(0, 0, 0);
    text1->scale(scale);
    text1->translate(20.0f * scale, 100.0f * scale);

    canvas->add(text1);

    auto text2 = tvg::Text::gen();
    text2->font("dejavu-sans-webfont");
    text2->text("from ThorVG + WebGL");
    text2->size(32);
    text2->fill(0, 0, 0);
    text2->scale(scale);
    text2->translate(20.0f * scale, 180.0f * scale);

    canvas->add(text2);

    canvas->draw();
    canvas->sync();
}

// ─────────────────────────────────────
static EM_BOOL wheelCallback(int, const EmscriptenWheelEvent *event, void *) {
    float factor = std::exp(-event->deltaY * 0.001f);

    zoom *= factor;

    redraw();

    return EM_TRUE;
}

// ─────────────────────────────────────
static int init() {
    if (tvg::Initializer::init() != tvg::Result::Success) {
        printf("ThorVG initialization failed\n");
        return -1;
    }

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);

    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.alpha = true;
    attrs.depth = false;
    attrs.stencil = true;
    attrs.antialias = true;

    glContext = emscripten_webgl_create_context("#canvas", &attrs);
    if (glContext <= 0) {
        printf("Failed to create WebGL2 context\n");
        return -1;
    }

    emscripten_webgl_make_context_current(glContext);

    double cssWidth = 0.0;
    double cssHeight = 0.0;

    emscripten_get_element_css_size("#canvas", &cssWidth, &cssHeight);

    dpr = emscripten_get_device_pixel_ratio();

    fbWidth = static_cast<uint32_t>(cssWidth * dpr);
    fbHeight = static_cast<uint32_t>(cssHeight * dpr);
    emscripten_set_canvas_element_size("#canvas", fbWidth, fbHeight);

    canvas = tvg::GlCanvas::gen();

    if (!canvas) {
        printf("Failed to create ThorVG GL canvas\n");
        return -1;
    }

    auto result = canvas->target(nullptr, nullptr, reinterpret_cast<void *>(glContext), 0, fbWidth,
                                 fbHeight, tvg::ColorSpace::ABGR8888S);

    if (result != tvg::Result::Success) {
        printf("canvas->target() failed: %d\n", (int)result);
        return -1;
    }

    result = canvas->viewport(0, 0, fbWidth, fbHeight);

    if (result != tvg::Result::Success) {
        printf("canvas->viewport() failed: %d\n", (int)result);
        return -1;
    }

    if (tvg::Text::load("dejavu-sans-webfont.ttf") != tvg::Result::Success) {
        printf("Failed to load font\n");
    }

    emscripten_set_wheel_callback("#canvas", nullptr, EM_TRUE, wheelCallback);

    redraw();

    return 0;
}

// ─────────────────────────────────────
int main() {
    if (init() != 0) {
        return -1;
    }

    printf("ThorVG WebGL initialized\n");

    return 0;
}
