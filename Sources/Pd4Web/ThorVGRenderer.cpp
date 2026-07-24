#include "ThorVGRenderer.hpp"

#include <GLES3/gl3.h>
#include <emscripten/threading.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

#ifndef NDEBUG
#define RENDER_LOG(...) emscripten_log(EM_LOG_CONSOLE, __VA_ARGS__)
#else
#define RENDER_LOG(...) ((void)0)
#endif

namespace {
constexpr float PixelsToPoints = 72.0f / 96.0f;

bool success(tvg::Result result) { return result == tvg::Result::Success; }

void releasePaint(tvg::Paint *paint) { tvg::Paint::rel(paint); }

template <typename T> using PendingPaint = std::unique_ptr<T, decltype(&releasePaint)>;

PendingPaint<tvg::Shape> shape() { return {tvg::Shape::gen(), releasePaint}; }

void parseColor(const char *hex, uint8_t &r, uint8_t &g, uint8_t &b) {
    r = g = b = 0;
    if (!hex) return;
    if (*hex == '#') ++hex;
    if (std::strlen(hex) < 6) return;
    char component[3] = {0, 0, 0};
    component[0] = hex[0]; component[1] = hex[1]; r = std::strtoul(component, nullptr, 16);
    component[0] = hex[2]; component[1] = hex[3]; g = std::strtoul(component, nullptr, 16);
    component[0] = hex[4]; component[1] = hex[5]; b = std::strtoul(component, nullptr, 16);
}

tvg::Matrix scaleMatrix(float scale) {
    return {scale, 0, 0, 0, scale, 0, 0, 0, 1};
}
}

ThorVGRenderer::~ThorVGRenderer() { shutdown(); }

void ThorVGRenderer::assertMainThread() const {
#ifndef NDEBUG
    assert(emscripten_is_main_runtime_thread() && "ThorVG called outside the main runtime thread");
#endif
}

bool ThorVGRenderer::initialize(const std::string &selector, const std::string &background,
                                const std::string &foreground, float zoom, int marginX,
                                int marginY) {
    assertMainThread();
    if (m_Initialized) return true;
    m_CanvasSelector = selector;
    m_Zoom = zoom;
    m_MarginX = marginX;
    m_MarginY = marginY;
    uint8_t r, g, b;
    parseColor(background.c_str(), r, g, b);
    m_BackgroundR = r / 255.0f;
    m_BackgroundG = g / 255.0f;
    m_BackgroundB = b / 255.0f;
    parseColor(foreground.c_str(), m_ForegroundR, m_ForegroundG, m_ForegroundB);

    if (!success(tvg::Initializer::init(0))) return false;
    m_EngineInitialized = true;

    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.majorVersion = 2;
    attributes.minorVersion = 0;
    attributes.alpha = false;
    attributes.depth = false;
    attributes.stencil = true;
    attributes.antialias = true;
    attributes.premultipliedAlpha = false;
    attributes.preserveDrawingBuffer = false;
    attributes.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    m_GlContext = emscripten_webgl_create_context(m_CanvasSelector.c_str(), &attributes);
    if (m_GlContext <= 0 || emscripten_webgl_make_context_current(m_GlContext) != EMSCRIPTEN_RESULT_SUCCESS) {
        shutdown();
        return false;
    }

    // ThorVG 1.1 documents SmartRender as ignored by GlCanvas. Keep the persistent
    // graph and explicitly record normal-render fallback instead of pretending it is active.
    m_Canvas = tvg::GlCanvas::gen(tvg::EngineOption::SmartRender);
    m_SmartRenderEnabled = false;
    if (!m_Canvas) {
        shutdown();
        return false;
    }
    resize();
    if (!success(m_Canvas->target(nullptr, nullptr, reinterpret_cast<void *>(m_GlContext), 0,
                                  m_FramebufferWidth, m_FramebufferHeight,
                                  tvg::ColorSpace::ABGR8888S)) ||
        !success(m_Canvas->viewport(0, 0, m_FramebufferWidth, m_FramebufferHeight))) {
        shutdown();
        return false;
    }
    m_ViewportDirty = false;

    m_RootScene = tvg::Scene::gen();
    if (!m_RootScene || !success(m_Canvas->add(m_RootScene))) {
        if (m_RootScene && !m_RootScene->parent()) tvg::Paint::rel(m_RootScene);
        m_RootScene = nullptr;
        shutdown();
        return false;
    }
    updateRootTransform();
    m_FontLoaded = success(tvg::Text::load("InterRegular.ttf"));
    if (!m_FontLoaded) {
        // Rendering continues; text commands will fail conversion and retain the old layer.
    }
    m_Initialized = true;
    RENDER_LOG("ThorVG initialized: backend=WebGL2 smartRender=%s",
               m_SmartRenderEnabled ? "enabled" : "unsupported/fallback");
    m_VisibleDirty = true;
    scheduleFrame();
    return true;
}

void ThorVGRenderer::shutdown() {
    if (!m_Canvas && !m_GlContext && !m_EngineInitialized) return;
    assertMainThread();
    m_Objects.clear();
    m_RootScene = nullptr;
    delete m_Canvas;
    m_Canvas = nullptr;
    if (m_GlContext > 0) emscripten_webgl_destroy_context(m_GlContext);
    m_GlContext = 0;
    if (m_FontLoaded) tvg::Text::unload("InterRegular.ttf");
    m_FontLoaded = false;
    if (m_EngineInitialized) tvg::Initializer::term();
    m_EngineInitialized = false;
    m_Initialized = false;
    m_FramePending = false;
}

void ThorVGRenderer::poll() {
    assertMainThread();
    if (RenderTransport::instance().hasPending()) scheduleFrame();
}

void ThorVGRenderer::scheduleFrame() {
    assertMainThread();
    if (!m_Initialized || m_FramePending) return;
    m_FramePending = true;
    auto &transport = RenderTransport::instance();
    RENDER_LOG("ThorVG frame scheduled: queue=%zu dropped=%llu", transport.depth(),
               static_cast<unsigned long long>(transport.dropped()));
    emscripten_request_animation_frame(renderFrame, this);
}

void ThorVGRenderer::setZoom(float zoom) {
    assertMainThread();
    if (zoom <= 0 || zoom == m_Zoom) return;
    m_Zoom = zoom;
    updateRootTransform();
    m_VisibleDirty = true;
    scheduleFrame();
}

void ThorVGRenderer::resize() {
    assertMainThread();
    if (m_CanvasSelector.empty()) return;
    double cssWidth = 0;
    double cssHeight = 0;
    if (emscripten_get_element_css_size(m_CanvasSelector.c_str(), &cssWidth, &cssHeight) !=
        EMSCRIPTEN_RESULT_SUCCESS) return;
    const double dpr = emscripten_get_device_pixel_ratio();
    const auto width = static_cast<uint32_t>(std::max(1.0, std::round(cssWidth * dpr)));
    const auto height = static_cast<uint32_t>(std::max(1.0, std::round(cssHeight * dpr)));
    if (width == m_FramebufferWidth && height == m_FramebufferHeight && dpr == m_DevicePixelRatio)
        return;
    m_FramebufferWidth = width;
    m_FramebufferHeight = height;
    m_DevicePixelRatio = dpr;
    emscripten_set_canvas_element_size(m_CanvasSelector.c_str(), width, height);
    m_ViewportDirty = true;
    updateRootTransform();
    m_VisibleDirty = true;
    if (m_Initialized) scheduleFrame();
}

void ThorVGRenderer::updateRootTransform() {
    if (!m_RootScene) return;
    m_RootScene->transform(scaleMatrix(m_Zoom * static_cast<float>(m_DevicePixelRatio)));
}

std::size_t ThorVGRenderer::layerCount() const noexcept {
    std::size_t count = 0;
    for (const auto &[id, object] : m_Objects) count += object.layers.size();
    return count;
}

EM_BOOL ThorVGRenderer::renderFrame(double, void *userData) {
    auto &renderer = *static_cast<ThorVGRenderer *>(userData);
    renderer.assertMainThread();
    renderer.m_FramePending = false;
    renderer.resize();

    auto &transport = RenderTransport::instance();
    while (const auto *transaction = transport.beginConsume()) {
        renderer.m_VisibleDirty |= renderer.apply(*transaction);
        transport.endConsume();
    }
    transport.markRecoveryReady();

    if (renderer.m_ViewportDirty) {
        emscripten_webgl_make_context_current(renderer.m_GlContext);
        if (success(renderer.m_Canvas->target(nullptr, nullptr,
                reinterpret_cast<void *>(renderer.m_GlContext), 0,
                renderer.m_FramebufferWidth, renderer.m_FramebufferHeight,
                tvg::ColorSpace::ABGR8888S))) {
            renderer.m_Canvas->viewport(0, 0, renderer.m_FramebufferWidth,
                                        renderer.m_FramebufferHeight);
        }
        renderer.m_ViewportDirty = false;
    }

    if (renderer.m_VisibleDirty) {
        emscripten_webgl_make_context_current(renderer.m_GlContext);
        // Scene::add/remove marks the child scene dirty, but ThorVG does not propagate
        // that status to its owning Canvas. Explicit update() is therefore required before
        // draw(); otherwise draw() returns InsufficientCondition after the first synced frame.
        if (success(renderer.m_Canvas->update())) {
            glViewport(0, 0, renderer.m_FramebufferWidth, renderer.m_FramebufferHeight);
            glClearColor(renderer.m_BackgroundR, renderer.m_BackgroundG,
                         renderer.m_BackgroundB, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            if (success(renderer.m_Canvas->draw(false)) &&
                success(renderer.m_Canvas->sync())) {
                RENDER_LOG("ThorVG frame rendered: objects=%zu layers=%zu queue=%zu dropped=%llu",
                           renderer.objectCount(), renderer.layerCount(), transport.depth(),
                           static_cast<unsigned long long>(transport.dropped()));
                renderer.m_VisibleDirty = false;
            }
        }
    }

    if (transport.hasPending() || renderer.m_VisibleDirty) renderer.scheduleFrame();
    return EM_FALSE;
}

ThorVGRenderer::ObjectNode *ThorVGRenderer::ensureObject(const LayerTransaction &transaction) {
    auto found = m_Objects.find(transaction.objectId);
    if (found != m_Objects.end()) return &found->second;
    auto *scene = tvg::Scene::gen();
    if (!scene) return nullptr;
    scene->translate(transaction.objectX - m_MarginX, transaction.objectY - m_MarginY);
    tvg::Paint *before = nullptr;
    ObjectId nextId = std::numeric_limits<ObjectId>::max();
    for (const auto &[id, object] : m_Objects) {
        if (id > transaction.objectId && id < nextId) {
            nextId = id;
            before = object.scene;
        }
    }
    if (!success(m_RootScene->add(scene, before))) {
        tvg::Paint::rel(scene);
        return nullptr;
    }
    ObjectNode node;
    node.scene = scene;
    node.x = transaction.objectX;
    node.y = transaction.objectY;
    node.width = transaction.objectWidth;
    node.height = transaction.objectHeight;
    return &m_Objects.emplace(transaction.objectId, std::move(node)).first->second;
}

bool ThorVGRenderer::apply(const LayerTransaction &transaction) {
    switch (transaction.type) {
    case RenderMessageType::ReplaceLayer:
        return replaceLayer(transaction);
    case RenderMessageType::RemoveLayer: {
        auto object = m_Objects.find(transaction.objectId);
        if (object == m_Objects.end()) return false;
        auto layer = object->second.layers.find(transaction.layerIndex);
        if (layer == object->second.layers.end()) return false;
        object->second.scene->remove(layer->second.scene);
        object->second.layers.erase(layer);
        return true;
    }
    case RenderMessageType::RemoveObject: {
        auto object = m_Objects.find(transaction.objectId);
        if (object == m_Objects.end()) return false;
        m_RootScene->remove(object->second.scene);
        m_Objects.erase(object);
        return true;
    }
    case RenderMessageType::ClearPatch:
        m_RootScene->remove();
        m_Objects.clear();
        return true;
    case RenderMessageType::UpdateObject: {
        auto object = m_Objects.find(transaction.objectId);
        if (object == m_Objects.end()) return false;
        object->second.x = transaction.objectX;
        object->second.y = transaction.objectY;
        object->second.width = transaction.objectWidth;
        object->second.height = transaction.objectHeight;
        object->second.scene->translate(transaction.objectX - m_MarginX,
                                        transaction.objectY - m_MarginY);
        return true;
    }
    }
    return false;
}

bool ThorVGRenderer::replaceLayer(const LayerTransaction &transaction) {
    auto *object = ensureObject(transaction);
    if (!object) return false;
    if (object->x != transaction.objectX || object->y != transaction.objectY) {
        object->x = transaction.objectX;
        object->y = transaction.objectY;
        object->scene->translate(object->x - m_MarginX, object->y - m_MarginY);
    }

    auto existing = object->layers.find(transaction.layerIndex);
    if (existing != object->layers.end() && existing->second.revision >= transaction.revision)
        return false;
    auto *replacement = buildLayer(transaction);
    if (!replacement) {
        emscripten_log(EM_LOG_WARN,
                       "ThorVG rejected layer transaction object=%llu layer=%d revision=%llu",
                       static_cast<unsigned long long>(transaction.objectId),
                       transaction.layerIndex,
                       static_cast<unsigned long long>(transaction.revision));
        return false;
    }

    tvg::Paint *before = nullptr;
    if (existing != object->layers.end()) before = existing->second.scene;
    else {
        const auto next = object->layers.upper_bound(transaction.layerIndex);
        if (next != object->layers.end()) before = next->second.scene;
    }
    if (!success(object->scene->add(replacement, before))) {
        tvg::Paint::rel(replacement);
        return false;
    }
    if (existing != object->layers.end()) object->scene->remove(existing->second.scene);
    object->layers[transaction.layerIndex] = {replacement, transaction.revision};
    RENDER_LOG("ThorVG layer applied: object=%llu layer=%d revision=%llu",
               static_cast<unsigned long long>(transaction.objectId), transaction.layerIndex,
               static_cast<unsigned long long>(transaction.revision));
    return true;
}

tvg::Scene *ThorVGRenderer::buildLayer(const LayerTransaction &transaction) {
    auto *layer = tvg::Scene::gen();
    if (!layer) return nullptr;
    for (const auto &command : transaction.commands) {
        auto *paint = createPaint(command, transaction);
        if (!paint || !success(layer->add(paint))) {
            if (paint && !paint->parent()) tvg::Paint::rel(paint);
            tvg::Paint::rel(layer);
            return nullptr;
        }
    }
    return layer;
}

tvg::Paint *ThorVGRenderer::createPaint(const RenderCommand &command,
                                        const LayerTransaction &transaction) {
    uint8_t r, g, b;
    parseColor(command.color, r, g, b);
    auto makeShape = [&]() { return shape(); };
    auto fillShape = [&](auto &paint) { return success(paint->fill(r, g, b, 255)); };
    auto strokeShape = [&](auto &paint) {
        return success(paint->fill(0, 0, 0, 0)) && success(paint->strokeFill(r, g, b, 255)) &&
               success(paint->strokeWidth(command.lineWidth));
    };

    switch (command.command) {
    case FILL_ALL: {
        PendingPaint<tvg::Scene> group{tvg::Scene::gen(), releasePaint};
        auto background = makeShape();
        auto border = makeShape();
        if (!group || !background || !border ||
            !success(background->appendRect(0, 0, command.canvasWidth,
                                            command.canvasHeight)) ||
            !fillShape(background))
            return nullptr;
        const float thickness = 1.0f / std::max(0.001f, m_Zoom);
        const float inset = thickness * 0.5f;
        if (!success(border->appendRect(inset, inset,
                                        std::max(0.0f, command.canvasWidth - thickness),
                                        std::max(0.0f, command.canvasHeight - thickness))) ||
            !success(border->fill(0, 0, 0, 0)) ||
            !success(border->strokeFill(m_ForegroundR, m_ForegroundG, m_ForegroundB, 255)) ||
            !success(border->strokeWidth(thickness)) ||
            !success(group->add(background.get())))
            return nullptr;
        background.release();
        if (!success(group->add(border.get()))) return nullptr;
        border.release();
        return group.release();
    }
    case FILL_RECT:
    case STROKE_RECT:
    case FILL_ROUNDED_RECT:
    case STROKE_ROUNDED_RECT: {
        auto paint = makeShape();
        if (!paint) return nullptr;
        float x = command.x1;
        float y = command.y1;
        float w = command.width;
        float h = command.height;
        float radius = (command.command == FILL_ROUNDED_RECT || command.command == STROKE_ROUNDED_RECT)
                           ? command.radius : 0;
        const bool stroked = command.command == STROKE_RECT || command.command == STROKE_ROUNDED_RECT;
        if (stroked) {
            const float inset = command.lineWidth * 0.5f;
            x += inset; y += inset; w = std::max(0.0f, w - command.lineWidth);
            h = std::max(0.0f, h - command.lineWidth); radius = std::max(0.0f, radius - inset);
        }
        if (!success(paint->appendRect(x, y, w, h, radius, radius)) ||
            !(stroked ? strokeShape(paint) : fillShape(paint))) return nullptr;
        return paint.release();
    }
    case FILL_ELLIPSE:
    case STROKE_ELLIPSE: {
        auto paint = makeShape();
        if (!paint || !success(paint->appendCircle(command.x1 + command.width * 0.5f,
             command.y1 + command.height * 0.5f, command.width * 0.5f, command.height * 0.5f)) ||
            !(command.command == STROKE_ELLIPSE ? strokeShape(paint) : fillShape(paint))) return nullptr;
        return paint.release();
    }
    case DRAW_LINE: {
        auto paint = makeShape();
        if (!paint || !success(paint->moveTo(command.x1, command.y1)) ||
            !success(paint->lineTo(command.x2, command.y2)) || !strokeShape(paint)) return nullptr;
        return paint.release();
    }
    case FILL_PATH:
    case STROKE_PATH: {
        auto paint = makeShape();
        if (!paint || command.pathOffset + command.pathCount > transaction.paths.size())
            return nullptr;
        float currentX = 0, currentY = 0;
        for (uint32_t i = 0; i < command.pathCount; ++i) {
            const auto &path = transaction.paths[command.pathOffset + i];
            tvg::Result result = tvg::Result::Unknown;
            switch (path.verb) {
            case RenderPathVerb::MoveTo:
                result = paint->moveTo(path.values[0], path.values[1]);
                currentX = path.values[0]; currentY = path.values[1];
                break;
            case RenderPathVerb::LineTo:
                result = paint->lineTo(path.values[0], path.values[1]);
                currentX = path.values[0]; currentY = path.values[1];
                break;
            case RenderPathVerb::QuadTo: {
                const float qx = path.values[0], qy = path.values[1];
                const float x = path.values[2], y = path.values[3];
                const auto cubic = QuadraticToCubic(currentX, currentY, qx, qy, x, y);
                result = paint->cubicTo(cubic.c1x, cubic.c1y, cubic.c2x, cubic.c2y,
                                        cubic.x, cubic.y);
                currentX = x; currentY = y;
                break;
            }
            case RenderPathVerb::CubicTo:
                result = paint->cubicTo(path.values[0], path.values[1], path.values[2],
                                        path.values[3], path.values[4], path.values[5]);
                currentX = path.values[4]; currentY = path.values[5];
                break;
            case RenderPathVerb::Close:
                result = paint->close();
                break;
            }
            if (!success(result)) return nullptr;
        }
        if (!(command.command == STROKE_PATH ? strokeShape(paint) : fillShape(paint))) return nullptr;
        return paint.release();
    }
    case DRAW_TEXT: {
        if (!command.text[0] || command.fontSize <= 0) return nullptr;
        PendingPaint<tvg::Text> text{tvg::Text::gen(), releasePaint};
        if (!text || !success(text->font("InterRegular")) || !success(text->text(command.text)) ||
            // Pd-Lua and NanoVG express text sizes in CSS pixels. ThorVG's Text::size()
            // accepts points and its SFNT loader converts them back with 96/72 DPI.
            !success(text->size(command.fontSize * PixelsToPoints)) ||
            !success(text->fill(r, g, b)) ||
            !success(text->align(0, 0))) return nullptr;
        if (command.width > 0) {
            if (!success(text->layout(command.width, 0)) || !success(text->wrap(tvg::TextWrap::Smart)))
                return nullptr;
        }
        text->translate(std::round(command.x1), std::round(command.y1));
        return text.release();
    }
    case DRAW_SVG: {
        if (!command.svgSize ||
            command.svgOffset + command.svgSize > transaction.svgBytes.size())
            return nullptr;
        PendingPaint<tvg::Picture> picture{tvg::Picture::gen(), releasePaint};
        if (!picture || !success(picture->load(transaction.svgBytes.data() + command.svgOffset,
             command.svgSize, "svg", nullptr, true))) return nullptr;
        picture->translate(command.x1, command.y1);
        return picture.release();
    }
    }
    return nullptr;
}
