#pragma once

#if __has_include("pd4web_config.h")
#include "pd4web_config.h"
#else
#include "config.h"
#endif
#include "RenderTransactionQueue.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <vector>

using ObjectId = uint64_t;

enum class RenderMessageType : uint8_t {
    ReplaceLayer,
    RemoveLayer,
    RemoveObject,
    ClearPatch,
    UpdateObject,
};

enum class RenderPathVerb : uint8_t { MoveTo, LineTo, QuadTo, CubicTo, Close };

struct RenderPathElement {
    RenderPathVerb verb = RenderPathVerb::MoveTo;
    float values[6]{};
};

struct CubicBezier {
    float c1x, c1y, c2x, c2y, x, y;
};

constexpr CubicBezier QuadraticToCubic(float p0x, float p0y, float qx, float qy,
                                       float p2x, float p2y) noexcept {
    return {p0x + (2.0f / 3.0f) * (qx - p0x),
            p0y + (2.0f / 3.0f) * (qy - p0y),
            p2x + (2.0f / 3.0f) * (qx - p2x),
            p2y + (2.0f / 3.0f) * (qy - p2y), p2x, p2y};
}

struct RenderCommand {
    LuaGuiCommands command = FILL_RECT;
    char color[10] = "#000000";
    float x1 = 0;
    float y1 = 0;
    float x2 = 0;
    float y2 = 0;
    float width = 0;
    float height = 0;
    float lineWidth = 0;
    float radius = 0;
    float fontSize = 0;
    float canvasWidth = 0;
    float canvasHeight = 0;
    uint32_t pathOffset = 0;
    uint32_t pathCount = 0;
    uint32_t svgOffset = 0;
    uint32_t svgSize = 0;
    char text[1024]{};
};

struct LayerTransaction {
    RenderMessageType type = RenderMessageType::ReplaceLayer;
    ObjectId objectId = 0;
    int32_t layerIndex = 0;
    uint64_t revision = 0;
    int32_t objectX = 0;
    int32_t objectY = 0;
    int32_t objectWidth = 0;
    int32_t objectHeight = 0;
    uint32_t objectOrder = 0;
    std::vector<RenderCommand> commands;
    std::vector<RenderPathElement> paths;
    std::vector<char> svgBytes;

    void reset(RenderMessageType messageType, ObjectId id, int layer) noexcept {
        type = messageType;
        objectId = id;
        layerIndex = layer;
        revision = 0;
        objectX = objectY = objectWidth = objectHeight = 0;
        objectOrder = 0;
        commands.clear();
        paths.clear();
        svgBytes.clear();
    }
};

class RenderTransport {
  public:
    LayerTransaction *beginLayer(ObjectId id, int layer, int x, int y, int w, int h) noexcept;
    bool append(const GuiCommand &command) noexcept;
    bool publishLayer() noexcept;
    bool publishLifecycle(RenderMessageType type, ObjectId id, int layer = -1) noexcept;
    bool publishObjectUpdate(ObjectId id, int x, int y, int w, int h) noexcept;

    const LayerTransaction *beginConsume() const noexcept { return m_Queue.beginPop(); }
    void endConsume() noexcept { m_Queue.commitPop(); }
    bool hasPending() const noexcept { return !m_Queue.empty(); }
    std::size_t depth() const noexcept { return m_Queue.depth(); }
    uint64_t dropped() const noexcept { return m_Dropped.load(std::memory_order_relaxed); }
    void markRecoveryReady() noexcept;
    bool takeRecovery(ObjectId &id, int &layer) noexcept;

  private:
    void requestRecovery(ObjectId id, int layer) noexcept;

    RenderUnboundedSpscQueue<LayerTransaction> m_Queue;
    LayerTransaction *m_Active = nullptr;
    bool m_ActiveValid = false;
    std::atomic<uint64_t> m_Dropped{0};
    std::atomic<uint64_t> m_NextRevision{1};
    std::atomic<ObjectId> m_RecoveryObject{0};
    std::atomic<int> m_RecoveryLayer{-1};
    // 0 = idle, 1 = requested by the audio producer, 2 = acknowledged by main.
    std::atomic<uint8_t> m_RecoveryState{0};
};

RenderTransport &GetRenderTransport() noexcept;
ObjectId AllocateRenderObjectId() noexcept;
