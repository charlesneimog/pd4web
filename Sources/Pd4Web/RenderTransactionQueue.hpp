#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

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
