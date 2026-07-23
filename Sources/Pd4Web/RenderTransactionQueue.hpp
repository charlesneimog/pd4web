#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>

// Wait-free bounded SPSC queue. Producer and consumer never touch the same slot.
template <typename T, std::size_t Capacity> class RenderSpscQueue {
    static_assert(Capacity > 1);

  public:
    T *beginPush() noexcept {
        const auto head = m_Head.load(std::memory_order_relaxed);
        const auto next = increment(head);
        if (next == m_Tail.load(std::memory_order_acquire)) return nullptr;
        return &m_Slots[head];
    }

    void commitPush() noexcept {
        m_Head.store(increment(m_Head.load(std::memory_order_relaxed)),
                     std::memory_order_release);
    }

    const T *beginPop() const noexcept {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if (tail == m_Head.load(std::memory_order_acquire)) return nullptr;
        return &m_Slots[tail];
    }

    void commitPop() noexcept {
        m_Tail.store(increment(m_Tail.load(std::memory_order_relaxed)),
                     std::memory_order_release);
    }

    bool empty() const noexcept {
        return m_Head.load(std::memory_order_acquire) ==
               m_Tail.load(std::memory_order_acquire);
    }

    std::size_t depth() const noexcept {
        const auto head = m_Head.load(std::memory_order_acquire);
        const auto tail = m_Tail.load(std::memory_order_acquire);
        return head >= tail ? head - tail : Capacity - tail + head;
    }

  private:
    static constexpr std::size_t increment(std::size_t value) noexcept {
        return (value + 1) % Capacity;
    }

    alignas(64) std::array<T, Capacity> m_Slots{};
    alignas(64) std::atomic<std::size_t> m_Head{0};
    alignas(64) std::atomic<std::size_t> m_Tail{0};
};

// Lossless single-producer/single-consumer queue. Nodes are allocated when the
// producer exceeds the previous high-water mark and recycled by the consumer.
// Consequently, queue capacity is limited only by available memory rather than
// by an arbitrary transaction count.
template <typename T> class RenderUnboundedSpscQueue {
  public:
    RenderUnboundedSpscQueue() noexcept {
        m_Head = m_Tail = new (std::nothrow) Node;
    }

    ~RenderUnboundedSpscQueue() {
        delete m_Pending;
        destroyList(m_Tail);
        destroyList(m_ProducerCache);
        destroyList(m_Recycled.exchange(nullptr, std::memory_order_relaxed));
    }

    RenderUnboundedSpscQueue(const RenderUnboundedSpscQueue &) = delete;
    RenderUnboundedSpscQueue &operator=(const RenderUnboundedSpscQueue &) = delete;

    T *beginPush() noexcept {
        if (m_Pending || !m_Head) return nullptr;
        m_Pending = acquireNode();
        return m_Pending ? &m_Pending->value : nullptr;
    }

    void cancelPush() noexcept {
        if (!m_Pending) return;
        recycleFromProducer(m_Pending);
        m_Pending = nullptr;
    }

    void commitPush() noexcept {
        if (!m_Pending || !m_Head) return;
        m_Pending->next.store(nullptr, std::memory_order_relaxed);
        m_Depth.fetch_add(1, std::memory_order_relaxed);
        m_Head->next.store(m_Pending, std::memory_order_release);
        m_Head = m_Pending;
        m_Pending = nullptr;
    }

    const T *beginPop() const noexcept {
        if (!m_Tail) return nullptr;
        auto *next = m_Tail->next.load(std::memory_order_acquire);
        return next ? &next->value : nullptr;
    }

    void commitPop() noexcept {
        if (!m_Tail) return;
        auto *next = m_Tail->next.load(std::memory_order_acquire);
        if (!next) return;
        auto *retired = m_Tail;
        m_Tail = next;
        recycleFromConsumer(retired);
        m_Depth.fetch_sub(1, std::memory_order_release);
    }

    bool empty() const noexcept { return beginPop() == nullptr; }

    std::size_t depth() const noexcept {
        return m_Depth.load(std::memory_order_acquire);
    }

  private:
    struct Node {
        std::atomic<Node *> next{nullptr};
        T value{};
    };

    Node *acquireNode() noexcept {
        if (!m_ProducerCache) {
            m_ProducerCache = m_Recycled.exchange(nullptr, std::memory_order_acquire);
        }
        if (m_ProducerCache) {
            auto *node = m_ProducerCache;
            m_ProducerCache = node->next.load(std::memory_order_relaxed);
            node->next.store(nullptr, std::memory_order_relaxed);
            return node;
        }
        return new (std::nothrow) Node;
    }

    void recycleFromProducer(Node *node) noexcept {
        node->next.store(m_ProducerCache, std::memory_order_relaxed);
        m_ProducerCache = node;
    }

    void recycleFromConsumer(Node *node) noexcept {
        auto *recycled = m_Recycled.load(std::memory_order_relaxed);
        do {
            node->next.store(recycled, std::memory_order_relaxed);
        } while (!m_Recycled.compare_exchange_weak(recycled, node, std::memory_order_release,
                                                    std::memory_order_relaxed));
    }

    static void destroyList(Node *node) noexcept {
        while (node) {
            auto *next = node->next.load(std::memory_order_relaxed);
            delete node;
            node = next;
        }
    }

    alignas(64) Node *m_Head = nullptr;
    Node *m_Pending = nullptr;
    Node *m_ProducerCache = nullptr;
    alignas(64) Node *m_Tail = nullptr;
    alignas(64) std::atomic<Node *> m_Recycled{nullptr};
    alignas(64) std::atomic<std::size_t> m_Depth{0};
};
