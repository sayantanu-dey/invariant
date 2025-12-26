#include <iostream>
#include <invariant/engine.hpp>
#include <unordered_map>

struct SimplePolicy {
    struct record {
        int key;
        int value;
    };
    using key_type = int;
    using record_type = record;
    using state_type = std::unordered_map<key_type, record_type>;

    static state_type create() {
        return state_type{};
    }

    static bool write(state_type& state, const record_type& r) {
        state[r.key] = r;
        return true;
    }

    static const record_type* query(const state_type& state, const key_type& k) {
        auto r = state.find(k);
        return r != state.cend() ? &r->second : nullptr;
    }
};

template <typename R>
struct NoHook {
    void pre_init() {}
    void post_init() {}
    void pre_write(const R&) {}
    void post_write(const R&) {}
};

int main() {
    using SimpleEngine = engine::Engine<SimplePolicy, NoHook<SimplePolicy::record_type>>;
    SimpleEngine e;
    e.put({1, 2});
    std::cout << "Simple Engine works value! for 1 is " << e.query(1)->value << std::endl;
    return 0;
}
