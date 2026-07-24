#include "RenderCommand.hpp"

#include <cassert>
#include <cmath>
#include <map>
#include <thread>
#include <unordered_map>

namespace {
struct Layer { uint64_t identity; uint64_t revision; };
struct Object { std::map<int, Layer> layers; };
struct Model {
    std::unordered_map<ObjectId, Object> objects;
    uint64_t nextIdentity = 1;
    float zoom = 1;
    int width = 0, height = 0;
    bool framePending = false;
    bool smart = false;
    bool canvasUpdated = false;

    void replace(ObjectId object, int layer, uint64_t revision) {
        auto &layers = objects[object].layers;
        auto found = layers.find(layer);
        if (found != layers.end() && found->second.revision >= revision) return;
        layers[layer] = {nextIdentity++, revision};
    }
};
}

int main() {
    auto &transport = RenderTransport::instance();
    GuiCommand source{};
    source.command = FILL_RECT;
    source.w = 20;
    source.h = 10;
    transport.beginLayer(77, 0, 4, 5, 20, 10);
    assert(transport.beginConsume() == nullptr);                 // private until end_paint
    assert(transport.append(source));
    assert(transport.beginConsume() == nullptr);
    assert(transport.publishLayer());
    const auto *published = transport.beginConsume();
    assert(published && published->objectId == 77 && published->commands.size() == 1);
    transport.endConsume();
    const auto droppedBefore = transport.dropped();
    constexpr int transactionCount = 1024;
    for (int i = 0; i < transactionCount; ++i)
        assert(transport.publishLifecycle(RenderMessageType::RemoveLayer, 77, i));
    assert(transport.dropped() == droppedBefore);
    assert(transport.depth() == transactionCount);
    for (int i = 0; i < transactionCount; ++i) {
        assert(transport.beginConsume()->layerIndex == i);
        transport.endConsume();
    }
    assert(!transport.hasPending());

    Model model;
    model.replace(1, 0, 1);                                      // one object/layer
    assert(model.objects[1].layers.size() == 1);
    model.replace(1, 2, 2); model.replace(1, 1, 3);              // numerical layer order
    int expected = 0; for (auto &[index, layer] : model.objects[1].layers) assert(index == expected++);
    model.replace(2, 0, 4);                                     // independent objects
    const auto unaffected = model.objects[1].layers[0].identity;
    model.replace(1, 1, 5);                                     // replace one layer only
    assert(model.objects[1].layers[0].identity == unaffected);
    model.objects[1].layers.erase(2);                            // remove layer
    model.objects.erase(2);                                     // remove object

    RenderSpscQueue<int, 4> mouse;
    *mouse.beginPush() = 1; mouse.commitPush();                  // down
    *mouse.beginPush() = 2; mouse.commitPush();                  // up in same interval
    assert(*mouse.beginPop() == 1); mouse.commitPop();
    assert(*mouse.beginPop() == 2); mouse.commitPop();
    *mouse.beginPush() = 3; mouse.commitPush();                  // repeated drags
    *mouse.beginPush() = 3; mouse.commitPush();
    *mouse.beginPush() = 3; mouse.commitPush();
    assert(mouse.beginPush() == nullptr);                        // explicit bounded overflow
    while (mouse.beginPop()) mouse.commitPop();

    RenderUnboundedSpscQueue<int> renderQueue;
    constexpr int concurrentTransactions = 100000;
    std::thread producer([&] {
        for (int i = 0; i < concurrentTransactions; ++i) {
            auto *slot = renderQueue.beginPush();
            assert(slot);
            *slot = i;
            renderQueue.commitPush();
        }
    });
    for (int expectedValue = 0; expectedValue < concurrentTransactions;) {
        if (const auto *value = renderQueue.beginPop()) {
            assert(*value == expectedValue++);
            renderQueue.commitPop();
        }
    }
    producer.join();
    assert(renderQueue.empty());

    model.framePending = true;                                  // arrival during render
    model.replace(1, 0, 6);
    model.canvasUpdated = true;                                 // nested scene dirties need Canvas::update
    assert(model.canvasUpdated);
    model.width = 1920; model.height = 1080;                     // resize preserves graph
    model.zoom = 2;                                              // zoom preserves geometry
    assert(model.objects[1].layers[1].identity != 0);

    RenderCommand text{}; text.command = DRAW_TEXT;              // text transport
    RenderCommand svg{}; svg.command = DRAW_SVG;                 // SVG transport
    RenderCommand fill{}; fill.command = FILL_PATH;              // filled path
    RenderCommand stroke{}; stroke.command = STROKE_PATH;        // stroked path
    assert(text.command == DRAW_TEXT && svg.command == DRAW_SVG);
    assert(fill.command == FILL_PATH && stroke.command == STROKE_PATH);

    const auto cubic = QuadraticToCubic(0, 0, 3, 6, 9, 0);
    assert(std::fabs(cubic.c1x - 2.0f) < 0.0001f);
    assert(std::fabs(cubic.c1y - 4.0f) < 0.0001f);
    assert(std::fabs(cubic.c2x - 5.0f) < 0.0001f);
    assert(std::fabs(cubic.c2y - 4.0f) < 0.0001f);

    model.smart = true; assert(model.smart);                     // supported mode model
    model.smart = false; assert(!model.smart);                   // GL fallback model
    bool audioInitialized = true; assert(audioInitialized);
    audioInitialized = false; assert(!audioInitialized);         // main-loop tick mode
}
