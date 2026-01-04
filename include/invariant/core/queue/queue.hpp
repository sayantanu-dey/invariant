#include <array>
#include <mutex>
#include <queue>
#include "spsc.hpp"

namespace invariant::queue::l {
    template<typename Task, size_t MAX_BUFFER=UINT8_MAX + 1>
    class Queue {
        static_assert(MAX_BUFFER > 1, "Buffer too small");
        static_assert((MAX_BUFFER & (MAX_BUFFER - 1)) == 0, "MAX_BUFFER must be power of two");
        static_assert(!std::is_pointer_v<Task>, "Queue cannot be used with raw pointers!");

        size_t _head{0};
        size_t _tail{0};
        std::array<Task, MAX_BUFFER> _buffer;

        std::mutex mtx;

    public:
        explicit Queue() = default;
        // For now I feel there is no need for below copy and moves, but yeah open to think
        Queue(Queue& q) = delete;
        Queue(const Queue& q) = delete;
        Queue(Queue&& q) = delete; 

        template<typename T>
        auto push(T&& event) noexcept {
            static_assert(std::is_convertible_v<T, Task>, 
                    "Error: The provided type cannot be converted to the required Task type.");
            std::lock_guard<std::mutex> lock(mtx);
            auto next = (_head + 1) & (MAX_BUFFER - 1);
            if (next == _tail) {
                return false; // queue full
            }
            _buffer[next] = std::forward<T>(event);
            _head = next;
            return true;
        }

        auto pop() noexcept -> std::optional<Task> {
            std::lock_guard<std::mutex> lock(mtx);
            if (_tail == _head) {
                return std::nullopt; // Queue empty
            }
            auto next_tail = (_tail + 1) & (MAX_BUFFER - 1);
            Task result = std::move(_buffer[next_tail]);
            _tail = next_tail;
            return result;
        }
    };
}

namespace invariant::queue::unb::l {
    template<typename Task>
    class Queue {
        static_assert(!std::is_pointer_v<Task>, "Queue cannot be used with raw pointers!");

        std::queue<Task> _queue;

        std::mutex mtx;

    public:
        explicit Queue() = default;
        // For now I feel there is no need for below copy and moves, but yeah open to think
        Queue(Queue& q) = delete;
        Queue(const Queue& q) = delete;
        Queue(Queue&& q) = delete; 

        template<typename T>
        auto push(T&& event) noexcept {
            static_assert(std::is_convertible_v<T, Task>, 
                    "Error: The provided type cannot be converted to the required Task type.");
            std::lock_guard<std::mutex> lock(mtx);
            _queue.push(event);
            return true;
        }

        auto pop() noexcept -> std::optional<Task> {
            std::lock_guard<std::mutex> lock(mtx);
            if (_queue.empty()) {
                return std::nullopt; // Queue empty
            }
            Task result = _queue.front();
            _queue.pop();
            return result;
        }
    };
}