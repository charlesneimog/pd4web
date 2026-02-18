#include "tvgRender.h"
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#include <thorvg.h>

#include <nanovg.h>
#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

#include <webgpu/webgpu_cpp.h>

// ------------------------------------------------------------
// Globals
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx1;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx2;

tvg::GlCanvas *tvgCanvas = nullptr;
NVGcontext *nvg = nullptr;

wgpu::Surface surface3;
static wgpu::Instance instance;
static wgpu::Device device;
tvg::WgCanvas *tvgCanvasWg = nullptr;

float zoomThor = 1.0f;
float zoomNano = 1.0f;
float zoomThorWg = 1.0f;

uint32_t fbWidth1, fbHeight1;
uint32_t fbWidth2, fbHeight2;
uint32_t fbWidth3, fbHeight3;

double dpr1, dpr2, dpr3;

// ------------------------------------------------------------
// ThorVG redraw

void redraw_thorvg() {
    emscripten_webgl_make_context_current(ctx1);

    glViewport(0, 0, fbWidth1, fbHeight1);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    std::list<tvg::Paint *> draws = tvgCanvas->paints();
    for (auto draw : draws) {
        tvgCanvas->remove(draw);
    }

    float totalScale = zoomThor * dpr1;

    auto text1 = tvg::Text::gen();
    text1->font("dejavu-sans-webfont");
    text1->text("Hello World");
    text1->size(80.0f);
    text1->fill(0, 0, 0);
    // text1->scale(totalScale);
    text1->translate(20.0f * totalScale, 20.0f * totalScale);
    tvgCanvas->add(text1);

    auto text2 = tvg::Text::gen();
    text2->font("dejavu-sans-webfont");
    text2->text("from thorvg");
    text2->size(20.0f);
    text2->fill(0, 0, 0);
    text2->scale(zoomThor);
    text2->translate(150.0f * totalScale, 150.0f * totalScale);
    tvgCanvas->add(text2);

    tvgCanvas->draw();
    tvgCanvas->sync();
}

// ------------------------------------------------------------
// NanoVG redraw

void redraw_nanovg() {
    emscripten_webgl_make_context_current(ctx2);

    glViewport(0, 0, fbWidth2, fbHeight2);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    nvgBeginFrame(nvg, fbWidth2, fbHeight2, dpr2);

    nvgScale(nvg, zoomNano, zoomNano);

    nvgFillColor(nvg, nvgRGBAf(0, 0, 0, 1));
    nvgFontSize(nvg, 90);
    nvgTextAlign(nvg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
    nvgTextBox(nvg, 20, 20, 800, "Hello World", nullptr);

    nvgFontSize(nvg, 20);
    nvgTextBox(nvg, 150, 150, 200, "from nanovg", nullptr);

    nvgEndFrame(nvg);
}

// ------------------------------------------------------------
// NanoVG redraw

void redraw_thorvg_webgpu() {
    if (!tvgCanvasWg) {
        std::cout << "ThorVG WebGPU canvas not initialized" << std::endl;
        return;
    }

    std::list<tvg::Paint *> draws = tvgCanvasWg->paints();
    for (auto draw : draws) {
        tvgCanvasWg->remove(draw);
    }

    auto rect = tvg::Shape::gen();
    rect->appendRect(0, 0, fbWidth3, fbHeight3, 0, 0, 0);
    rect->fill(255, 255, 255);
    tvgCanvasWg->add(rect);

    auto text1 = tvg::Text::gen();
    text1->font("dejavu-sans-webfont");
    text1->text("Hello World");
    text1->size(80.0f);
    text1->fill(0, 0, 0);
    text1->scale(zoomThorWg);
    text1->translate(20.0f * zoomThorWg, 20.0f * zoomThorWg);
    tvgCanvasWg->add(text1);

    auto text2 = tvg::Text::gen();
    text2->font("dejavu-sans-webfont");
    text2->text("from thorvg (webgpu)");
    text2->size(20.0f);
    text2->fill(0, 0, 0);
    text2->scale(zoomThorWg);
    text2->translate(150.0f * zoomThorWg, 150.0f * zoomThorWg);
    tvgCanvasWg->add(text2);

    tvgCanvasWg->draw();
    tvgCanvasWg->sync();
}

// ------------------------------------------------------------
// Wheel callbacks
EM_BOOL wheel_thor(int, const EmscriptenWheelEvent *e, void *) {
    float factor = std::exp(-e->deltaY * 0.001f);
    zoomThor *= factor;
    redraw_thorvg();
    return EM_TRUE;
}

EM_BOOL wheel_nano(int, const EmscriptenWheelEvent *e, void *) {
    float factor = std::exp(-e->deltaY * 0.001f);
    zoomNano *= factor;
    redraw_nanovg();
    return EM_TRUE;
}

EM_BOOL wheel_thor_wg(int, const EmscriptenWheelEvent *e, void *) {
    float factor = std::exp(-e->deltaY * 0.001f);
    zoomThorWg *= factor;
    redraw_thorvg_webgpu();
    return EM_TRUE;
}

// ------------------------------------------------------------
// Init ThorVG

int init_thorvg() {
    if (tvg::Initializer::init(0) != tvg::Result::Success) {
        return -1;
    }

    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.stencil = true;
    attr.antialias = true;

    ctx1 = emscripten_webgl_create_context("#canvas1", &attr);
    if (ctx1 <= 0) {
        return -1;
    }

    emscripten_webgl_make_context_current(ctx1);

    // Get CSS size
    double cssW, cssH;
    emscripten_get_element_css_size("#canvas1", &cssW, &cssH);

    // Get device pixel ratio
    dpr1 = emscripten_get_device_pixel_ratio();

    // Compute framebuffer size in device pixels
    fbWidth1 = static_cast<int>(cssW * dpr1);
    fbHeight1 = static_cast<int>(cssH * dpr1);

    // Resize the canvas to match framebuffer size
    emscripten_set_canvas_element_size("#canvas1", fbWidth1, fbHeight1);

    // Initialize ThorVG GL canvas
    tvgCanvas = tvg::GlCanvas::gen();
    tvgCanvas->target(nullptr, nullptr, reinterpret_cast<void *>(static_cast<uintptr_t>(ctx1)), 0,
                      fbWidth1, fbHeight1, tvg::ColorSpace::ABGR8888S);
    tvgCanvas->viewport(0, 0, fbWidth1, fbHeight1);

    // Load font
    tvg::Text::load("dejavu-sans-webfont.ttf");




    // Wheel callback
    emscripten_set_wheel_callback("#canvas1", nullptr, EM_TRUE, wheel_thor);

    // Initial redraw
    redraw_thorvg();

    printf("Canvas CSS: %f x %f, DPR: %f, framebuffer: %d x %d\n", cssW, cssH, dpr1, fbWidth1,
           fbHeight1);

    return 0;
}

// ------------------------------------------------------------
// Init NanoVG

int init_nanovg() {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.stencil = true;
    attr.antialias = false;

    ctx2 = emscripten_webgl_create_context("#canvas2", &attr);
    if (ctx2 <= 0) {
        return -1;
    }

    emscripten_webgl_make_context_current(ctx2);

    double cssW, cssH;
    emscripten_get_element_css_size("#canvas2", &cssW, &cssH);
    dpr2 = emscripten_get_device_pixel_ratio();
    fbWidth2 = cssW * dpr2;
    fbHeight2 = cssH * dpr2;
    emscripten_set_canvas_element_size("#canvas2", fbWidth2, fbHeight2);

    nvg = nvglCreate(NVG_AUTOW_DEFAULT);
    nvgCreateFont(nvg, "dejavu-sans-webfont", "dejavu-sans-webfont.ttf");

    emscripten_set_wheel_callback("#canvas2", nullptr, EM_TRUE, wheel_nano);

    redraw_nanovg();
    return 0;
}

// ------------------------------------------------------------
// Init WebGPU

void init_webgpu_canvas(const char *selector, uint32_t width, uint32_t height) {
    if (!selector) {
        std::cout << "No selector" << std::endl;
        return;
    };

    // Resolve framebuffer size from CSS size * device pixel ratio.
    double cssW = 0.0, cssH = 0.0;
    emscripten_get_element_css_size(selector, &cssW, &cssH);
    dpr3 = emscripten_get_device_pixel_ratio();

    if (width == 0) {
        width = static_cast<uint32_t>(cssW * dpr3);
    }
    if (height == 0) {
        height = static_cast<uint32_t>(cssH * dpr3);
    }

    fbWidth3 = width;
    fbHeight3 = height;
    emscripten_set_canvas_element_size(selector, fbWidth3, fbHeight3);

    // Create an instance only once. We use WaitAny for synchronous adapter/device requests.
    if (!instance) {
        wgpu::InstanceDescriptor desc{};
        static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &kTimedWaitAny;
        instance = wgpu::CreateInstance(&desc);
        assert(instance);
    }

    // Request device once.
    if (!device) {
        wgpu::RequestAdapterWebXROptions xrOptions = {};
        wgpu::RequestAdapterOptions options = {};
        options.nextInChain = &xrOptions;

        wgpu::Adapter adapter;
        wgpu::Future f1 = instance.RequestAdapter(
            &options, wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::RequestAdapterStatus status, wgpu::Adapter ad, wgpu::StringView message) {
                if (message.length) {
                    std::printf("RequestAdapter: %.*s\n", (int)message.length, message.data);
                }
                if (status == wgpu::RequestAdapterStatus::Unavailable) {
                    std::printf("WebGPU unavailable; exiting cleanly\n");
                    std::exit(0);
                }
                assert(status == wgpu::RequestAdapterStatus::Success);
                adapter = std::move(ad);
            });
        instance.WaitAny(f1, UINT64_MAX);
        assert(adapter);

        wgpu::Limits limits{};
        wgpu::DeviceDescriptor devDesc{};
        devDesc.requiredLimits = &limits;
        devDesc.SetUncapturedErrorCallback(
            [](const wgpu::Device &, wgpu::ErrorType errorType, wgpu::StringView message) {
                if (message.length) {
                    std::printf("UncapturedError (type=%d): %.*s\n", (int)errorType,
                                (int)message.length, message.data);
                }
            });
        devDesc.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device &, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                if (message.length) {
                    std::printf("DeviceLost (reason=%d): %.*s\n", (int)reason, (int)message.length,
                                message.data);
                }
            });

        wgpu::Future f2 = adapter.RequestDevice(
            &devDesc, wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::RequestDeviceStatus status, wgpu::Device dev, wgpu::StringView message) {
                if (message.length) {
                    std::printf("RequestDevice: %.*s\n", (int)message.length, message.data);
                }
                assert(status == wgpu::RequestDeviceStatus::Success);
                device = std::move(dev);
            });
        instance.WaitAny(f2, UINT64_MAX);
        assert(device);
    }

    // Create / re-create surface bound to the given canvas selector.
    {
        wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = selector;

        wgpu::SurfaceDescriptor surfDesc{};
        surfDesc.nextInChain = &canvasDesc;
        surface3 = instance.CreateSurface(&surfDesc);
        assert(surface3);
    }
}

// ------------------------------------------------------------
// Init ThorVG (WebGPU)

int init_thorvg_webgpu() {
    init_webgpu_canvas("#canvas3", 0, 0);

    if (!instance || !device || !surface3) {
        return -1;
    }

    std::cout << "WebGPU is Ok" << std::endl;
    tvgCanvasWg = tvg::WgCanvas::gen(tvg::EngineOption::SmartRender);
    if (!tvgCanvasWg) {
        std::cerr << "Not possible create canvas" << std::endl;
        return -1;
    }

    std::cout << "WebGPU canvas is Ok" << std::endl;
    auto res = tvgCanvasWg->target(reinterpret_cast<void *>(device.Get()),
                                   reinterpret_cast<void *>(instance.Get()),
                                   reinterpret_cast<void *>(surface3.Get()), fbWidth3, fbHeight3,
                                   tvg::ColorSpace::ABGR8888S, 0);
    if (res != tvg::Result::Success) {
        std::printf("ThorVG WgCanvas::target failed: %d\n", (int)res);
        return -1;
    }

    // Font is already loaded in init_thorvg(), but safe to call again.
    tvg::Text::load("dejavu-sans-webfont.ttf");
    emscripten_set_wheel_callback("#canvas3", nullptr, EM_TRUE, wheel_thor_wg);

    redraw_thorvg_webgpu();
    return 0;
}

// ------------------------------------------------------------
int main() {
    if (tvg::Initializer::init(0) != tvg::Result::Success) {
        return -1;
    }

    if (init_thorvg() != 0) {
        return -1;
    }
    if (init_nanovg() != 0) {
        return -1;
    }
    if (init_thorvg_webgpu() != 0) {
        return -1;
    }

    std::cout << "Everything is ok" << std::endl;
    return 0;
}
