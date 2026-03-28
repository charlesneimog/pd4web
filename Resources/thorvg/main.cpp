#include <thorvg.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <cmath>
#include <iostream>

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx1 = 0;
tvg::GlCanvas *tvgCanvas = nullptr;
float zoomThor = 1.0f;
uint32_t fbWidth1, fbHeight1;
double dpr1;

void redraw_thorvg() {
    if (!tvgCanvas) {
        return;
    }

    printf("redraw\n");
    emscripten_webgl_make_context_current(ctx1);

    glViewport(0, 0, fbWidth1, fbHeight1);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    auto paints = tvgCanvas->paints();
    for (auto p : paints) {
        tvgCanvas->remove(p);
    }

    float totalScale = zoomThor * dpr1;

    auto text1 = tvg::Text::gen();
    text1->font("dejavu-sans-webfont");
    text1->text("Hello World");
    text1->size(80);
    text1->fill(0, 0, 0);
    text1->scale(totalScale);
    text1->translate(20.0f * totalScale, 20.0f * totalScale);
    tvgCanvas->add(text1);

    auto text2 = tvg::Text::gen();
    text2->font("dejavu-sans-webfont");
    text2->text("from ThorVG");
    text2->size(20);
    text2->fill(0, 0, 0);
    text2->scale(totalScale);
    text2->translate(150.0f * totalScale, 150.0f * totalScale);
    tvgCanvas->add(text2);

    tvgCanvas->draw();
    tvgCanvas->sync();
}

EM_BOOL wheel_thor(int, const EmscriptenWheelEvent *e, void *) {
    float factor = std::exp(-e->deltaY * 0.001f);
    zoomThor *= factor;
    redraw_thorvg();
    return EM_TRUE;
}

int init_thorvg_webgl() {
    if (tvg::Initializer::init(0) != tvg::Result::Success) {
        printf("ThorVG init failed\n");
        return -1;
    }

    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.stencil = true;
    attr.antialias = true;

    ctx1 = emscripten_webgl_create_context("#canvas1", &attr);
    if (ctx1 <= 0) {
        printf("WebGL context creation failed\n");
        return -1;
    }
    emscripten_webgl_make_context_current(ctx1);

    double cssW, cssH;
    emscripten_get_element_css_size("#canvas1", &cssW, &cssH);
    dpr1 = emscripten_get_device_pixel_ratio();
    fbWidth1 = static_cast<int>(cssW * dpr1);
    fbHeight1 = static_cast<int>(cssH * dpr1);
    emscripten_set_canvas_element_size("#canvas1", fbWidth1, fbHeight1);

    tvgCanvas = tvg::GlCanvas::gen();
    if (!tvgCanvas) {
        printf("not possible to create canvas\n");
        return -1;
    }

    auto res = tvgCanvas->target(nullptr, // display
                                 nullptr, // surface
                                 reinterpret_cast<void *>(ctx1), 0, fbWidth1, fbHeight1,
                                 tvg::ColorSpace::ABGR8888S);
    if (res != tvg::Result::Success) {
        printf("GlCanvas::target failed: %d\n", (int)res);
        return -1;
    }

    res = tvgCanvas->viewport(0, 0, fbWidth1, fbHeight1);
    if (res != tvg::Result::Success) {
        printf("GlCanvas::viewport failed: %d\n", (int)res);
        return -1;
    }

    if (tvg::Text::load("dejavu-sans-webfont.ttf") != tvg::Result::Success) {
        printf("Font load failed\n");
    }

    emscripten_set_wheel_callback("#canvas1", nullptr, EM_TRUE, wheel_thor);

    redraw_thorvg();
    return 0;
}

int main() {
    if (init_thorvg_webgl() != 0) {
        return -1;
    }
    printf("ThorVG WebGL initialized\n");
    return 0;
}
