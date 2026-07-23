#include "RenderCommand.hpp"

#include <algorithm>

namespace {
std::atomic<ObjectId> NextObjectId{1};
RenderTransport Transport;
}

RenderTransport &GetRenderTransport() noexcept { return Transport; }

ObjectId AllocateRenderObjectId() noexcept {
    return NextObjectId.fetch_add(1, std::memory_order_relaxed);
}

void RenderTransport::requestRecovery(ObjectId id, int layer) noexcept {
    if (m_RecoveryState.load(std::memory_order_acquire) != 0) return;
    m_RecoveryObject.store(id, std::memory_order_relaxed);
    m_RecoveryLayer.store(layer, std::memory_order_relaxed);
    uint8_t idle = 0;
    m_RecoveryState.compare_exchange_strong(idle, 1, std::memory_order_release,
                                            std::memory_order_relaxed);
}

LayerTransaction *RenderTransport::beginLayer(ObjectId id, int layer, int x, int y, int w,
                                               int h) noexcept {
    if (m_Active) {
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }
    m_Active = m_Queue.beginPush();
    if (!m_Active) {
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        requestRecovery(id, layer);
        return nullptr;
    }
    m_Active->reset(RenderMessageType::ReplaceLayer, id, layer);
    m_ActiveValid = true;
    m_Active->objectX = x;
    m_Active->objectY = y;
    m_Active->objectWidth = w;
    m_Active->objectHeight = h;
    return m_Active;
}

bool RenderTransport::append(const GuiCommand &source) noexcept {
    if (!m_Active) {
        m_ActiveValid = false;
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    m_Active->commands.emplace_back();
    auto &command = m_Active->commands.back();
    command.command = source.command;
    std::strncpy(command.color, source.current_color, sizeof(command.color) - 1);
    command.x1 = source.x1;
    command.y1 = source.y1;
    command.x2 = source.x2;
    command.y2 = source.y2;
    command.width = source.w;
    command.height = source.h;
    command.lineWidth = source.line_width;
    command.radius = source.radius;
    command.fontSize = source.font_size;
    command.canvasWidth = source.canvas_width;
    command.canvasHeight = source.canvas_height;
    std::memcpy(command.text, source.text, sizeof(command.text));
    command.text[sizeof(command.text) - 1] = '\0';

    if (source.path_elements && source.path_element_count > 0) {
        const auto count = static_cast<uint32_t>(source.path_element_count);
        command.pathOffset = static_cast<uint32_t>(m_Active->paths.size());
        command.pathCount = count;
        m_Active->paths.reserve(m_Active->paths.size() + count);
        for (uint32_t i = 0; i < count; ++i) {
            const auto &input = source.path_elements[i];
            m_Active->paths.emplace_back();
            auto &element = m_Active->paths.back();
            switch (input.verb) {
            case LUA_PATH_MOVE_TO: element.verb = RenderPathVerb::MoveTo; break;
            case LUA_PATH_LINE_TO: element.verb = RenderPathVerb::LineTo; break;
            case LUA_PATH_QUAD_TO: element.verb = RenderPathVerb::QuadTo; break;
            case LUA_PATH_CUBIC_TO: element.verb = RenderPathVerb::CubicTo; break;
            case LUA_PATH_CLOSE: element.verb = RenderPathVerb::Close; break;
            }
            std::memcpy(element.values, input.values, sizeof(element.values));
        }
    } else if (source.path_coords && source.path_size > 0) {
        const auto count = static_cast<uint32_t>(source.path_size);
        command.pathOffset = static_cast<uint32_t>(m_Active->paths.size());
        command.pathCount = count;
        m_Active->paths.reserve(m_Active->paths.size() + count);
        for (uint32_t i = 0; i < count; ++i) {
            m_Active->paths.emplace_back();
            auto &element = m_Active->paths.back();
            element.verb = i == 0 ? RenderPathVerb::MoveTo : RenderPathVerb::LineTo;
            element.values[0] = source.path_coords[i * 2];
            element.values[1] = source.path_coords[i * 2 + 1];
        }
    }

    if (source.command == DRAW_SVG && source.svg) {
        const auto length = std::strlen(source.svg);
        if (length == 0) {
            m_ActiveValid = false;
            m_Dropped.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        command.svgOffset = static_cast<uint32_t>(m_Active->svgBytes.size());
        command.svgSize = static_cast<uint32_t>(length);
        m_Active->svgBytes.insert(m_Active->svgBytes.end(), source.svg, source.svg + length + 1);
    }

    return true;
}

bool RenderTransport::publishLayer() noexcept {
    if (!m_Active) return false;
    if (!m_ActiveValid) {
        requestRecovery(m_Active->objectId, m_Active->layerIndex);
        m_Queue.cancelPush();
        m_Active = nullptr;
        m_ActiveValid = false;
        return false;
    }
    m_Active->revision = m_NextRevision.fetch_add(1, std::memory_order_relaxed);
    m_Active = nullptr;
    m_ActiveValid = false;
    m_Queue.commitPush();
    return true;
}

void RenderTransport::markRecoveryReady() noexcept {
    uint8_t requested = 1;
    m_RecoveryState.compare_exchange_strong(requested, 2, std::memory_order_release,
                                            std::memory_order_relaxed);
}

bool RenderTransport::takeRecovery(ObjectId &id, int &layer) noexcept {
    uint8_t ready = 2;
    if (!m_RecoveryState.compare_exchange_strong(ready, 0, std::memory_order_acq_rel,
                                                  std::memory_order_relaxed))
        return false;
    id = m_RecoveryObject.load(std::memory_order_relaxed);
    layer = m_RecoveryLayer.load(std::memory_order_relaxed);
    return id != 0;
}

bool RenderTransport::publishLifecycle(RenderMessageType type, ObjectId id, int layer) noexcept {
    if (m_Active) {
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    auto *message = m_Queue.beginPush();
    if (!message) {
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    message->reset(type, id, layer);
    message->revision = m_NextRevision.fetch_add(1, std::memory_order_relaxed);
    m_Queue.commitPush();
    return true;
}

bool RenderTransport::publishObjectUpdate(ObjectId id, int x, int y, int w, int h) noexcept {
    if (m_Active) {
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    auto *message = m_Queue.beginPush();
    if (!message) {
        m_Dropped.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    message->reset(RenderMessageType::UpdateObject, id, -1);
    message->revision = m_NextRevision.fetch_add(1, std::memory_order_relaxed);
    message->objectX = x;
    message->objectY = y;
    message->objectWidth = w;
    message->objectHeight = h;
    m_Queue.commitPush();
    return true;
}
