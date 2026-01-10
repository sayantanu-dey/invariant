#include <atomic>
#include <thread>
#include <type_traits>
#include <array>
#include <mutex>
#include <new>

namespace invariant::queue::lfree::spsc {
    template<typename Task, size_t MAX_BUFFER=UINT8_MAX + 1>
    class Queue {
        static_assert(MAX_BUFFER > 1, "Buffer too small");
        static_assert((MAX_BUFFER & (MAX_BUFFER - 1)) == 0, "MAX_BUFFER must be power of two");
        static_assert(!std::is_pointer_v<Task>, "Queue cannot be used with raw pointers!");

        static constexpr size_t CACHE_LINE_SIZE = 
            #ifdef __cpp_lib_hardware_interference_size
                std::hardware_destructive_interference_size
            #else
                128  // Fallback for Apple Silicon and other platforms
            #endif
        ;

        alignas(CACHE_LINE_SIZE) std::atomic<size_t> _head{0};
        alignas(CACHE_LINE_SIZE) std::atomic<size_t> _tail{0};
        
        // Align buffer to cache line to separate from head/tail
        alignas(CACHE_LINE_SIZE) std::array<Task, MAX_BUFFER> _buffer;

    public:
        explicit Queue() = default;
        Queue(Queue& q) = delete;
        Queue(const Queue& q) = delete;
        Queue(Queue&& q) = delete;

        template<typename T>
        auto push(T&& event) noexcept {
            static_assert(std::is_convertible_v<T, Task>, 
                    "Error: The provided type cannot be converted to the required Task type.");
            
            auto head = _head.load(std::memory_order_relaxed);
            auto next = (head + 1) & (MAX_BUFFER - 1);
            // Must acquire to see consumer's tail updates
            if (next == _tail.load(std::memory_order_acquire)) {
                return false;
            }
            _buffer[next] = std::forward<T>(event);
            _head.store(next, std::memory_order_release);
            return true;
        }

        auto pop() noexcept -> std::optional<Task> {
            auto tail = _tail.load(std::memory_order_relaxed);
            auto head = _head.load(std::memory_order_acquire);
            if (tail == head) {
                return std::nullopt;
            }
            auto next_tail = (tail + 1) & (MAX_BUFFER - 1);
            Task result = std::move(_buffer[next_tail]);
            _tail.store(next_tail, std::memory_order_release);
            return result;
        }
    };
}