#ifndef ENGINE_ENGINE_HPP
#define ENGINE_ENGINE_HPP
#include <concepts>
#include <tuple>
#include <utility>
#include <vector>

namespace engine {
    /**
     * Engine Policy declares that a valid Policy should define how a record is applied to a collection.
     */
    template <typename P>
    concept EnginePolicy =
    requires(
        typename P::state_type&       st,
        const typename P::state_type& cst,
        const typename P::record_type& rec,
        const typename P::key_type&    key
    ) {
        typename P::state_type;
        typename P::record_type;
        typename P::key_type;
        { P::create() } -> std::same_as<typename P::state_type>;
        { P::write(st, rec) } -> std::same_as<bool>;
        { P::query(cst, key) } -> std::same_as<const typename P::record_type*>;
    };

    /**
     * Engine Hook declares that a valid Hook operating inside the engine should define certain set of functions
     *  
     * * pre_init: this is called right before the state store is initialized or loaded.
     * * post_init: this is called right after the state is restored
     * * pre_write: this is called before writing any record to the store
     * * post_write: this is called after writing any record to the store 
     */
    template <typename H, typename R>
    concept EngineHook = 
    requires (H h, const R& r, const std::vector<R>& rv) {
        {h.pre_init()} -> std::same_as<void>;
        {h.post_init()} -> std::same_as<void>;
        {h.pre_write(r)} -> std::same_as<void>;
        {h.post_write(r)} -> std::same_as<void>;
    };

    /**
     * SeqHookChain, this struct helps to chain multiple Hooks serially
     */
    template<typename Record, typename... Hooks>
    requires (EngineHook<Hooks, Record> && ...)
    struct SeqHookChain {
        SeqHookChain() = default;
        explicit SeqHookChain(Hooks... hs)
            : hooks_(std::move(hs)...) {}
#define REGISTER_HOOK_FUNC(NAME, PARAMS, CALL_ARGS)     \
    void NAME PARAMS {                                  \
        std::apply(                                     \
            [&](auto&... hs) {                          \
                (hs.NAME CALL_ARGS, ...);               \
            },                                          \
            hooks_                                      \
        );                                              \
    }                                                   
        REGISTER_HOOK_FUNC(pre_init, (), ());
        REGISTER_HOOK_FUNC(post_init, (), ());
        REGISTER_HOOK_FUNC(pre_write, (const Record& record), (record));
        REGISTER_HOOK_FUNC(post_write, (const Record& record), (record));
#undef REGISTER_HOOK_FUNC
    private:
        std::tuple<Hooks...> hooks_;
    };

#define EngineTemplate template<typename Policy, typename Hook>                 \
requires EnginePolicy<Policy> && EngineHook<Hook, typename Policy::record_type> 

    /**
     * Actual Engine template, this is responsible to create/load the state, and serve queries to read and write to the
     * state. 
     */
    EngineTemplate
    class Engine {
    public:
        using state_type = typename Policy::state_type;
        using record_type = typename Policy::record_type;
        using key_type = typename Policy::key_type;
        explicit Engine() : _state(Policy::create()) {};
        explicit Engine(Hook hook) : _state(Policy::create()), _hook(std::move(hook)) {};
        
        /**
         * Query a record using its key
         */
        const record_type* query(const key_type& key) const {
            return Policy::query(_state, key);
        };
        
        /**
         * Put a record into the state
         */
        bool put(const record_type record) {
            _hook.pre_write(record);
            bool ok = Policy::write(_state, record);
            _hook.post_write(record);
            return ok;
        };
    private:
        state_type _state;
        Hook _hook;
    };
}
#endif