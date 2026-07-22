#pragma once

#include "RenderCommand.hpp"

#include <emscripten/html5.h>
#include <thorvg.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

class ThorVGRenderer {
  public:
    ThorVGRenderer() = default;
    ~ThorVGRenderer();
    ThorVGRenderer(const ThorVGRenderer &) = delete;
    ThorVGRenderer &operator=(const ThorVGRenderer &) = delete;

    bool initialize(const std::string &canvasSelector, const std::string &background,
                    const std::string &foreground, float zoom, int marginX, int marginY);
    void shutdown();
    void poll();
    void scheduleFrame();
    void setZoom(float zoom);
    void resize();

    bool initialized() const noexcept { return m_Initialized; }
    bool smartRenderEnabled() const noexcept { return m_SmartRenderEnabled; }
    std::size_t objectCount() const noexcept { return m_Objects.size(); }
    std::size_t layerCount() const noexcept;

  private:
    struct LayerNode {
        tvg::Scene *scene = nullptr; // Owned by ObjectNode::scene.
        uint64_t revision = 0;
    };
    struct ObjectNode {
        tvg::Scene *scene = nullptr; // Owned by root scene.
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        std::map<int, LayerNode> layers;
    };

    static EM_BOOL renderFrame(double time, void *userData);
    bool apply(const LayerTransaction &transaction);
    bool replaceLayer(const LayerTransaction &transaction);
    ObjectNode *ensureObject(const LayerTransaction &transaction);
    tvg::Scene *buildLayer(const LayerTransaction &transaction);
    tvg::Paint *createPaint(const RenderCommand &command,
                            const LayerTransaction &transaction);
    void updateRootTransform();
    void assertMainThread() const;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_GlContext = 0;
    tvg::GlCanvas *m_Canvas = nullptr;
    tvg::Scene *m_RootScene = nullptr; // Owned by m_Canvas.
    std::unordered_map<ObjectId, ObjectNode> m_Objects;
    std::string m_CanvasSelector;
    uint32_t m_FramebufferWidth = 0;
    uint32_t m_FramebufferHeight = 0;
    double m_DevicePixelRatio = 1.0;
    float m_Zoom = 1.0f;
    int m_MarginX = 0;
    int m_MarginY = 0;
    float m_BackgroundR = 1.0f;
    float m_BackgroundG = 1.0f;
    float m_BackgroundB = 1.0f;
    uint8_t m_ForegroundR = 0;
    uint8_t m_ForegroundG = 0;
    uint8_t m_ForegroundB = 0;
    bool m_Initialized = false;
    bool m_EngineInitialized = false;
    bool m_FontLoaded = false;
    bool m_FramePending = false;
    bool m_ViewportDirty = false;
    bool m_VisibleDirty = false;
    bool m_SmartRenderEnabled = false;
};
