#ifndef WAL_HPP
#define WAL_HPP
#include <memory>
#include <invariant/engine.hpp>

namespace WAL {

    template <typename Record>
    class WAL {
        WAL() : _filename("wal") {}
        WAL(const std::string& filename) : _filename(filename) {}    
        void pre_init() {}
        void pos_init(const engine::Config& config) {
            namespace fs = std::filesystem;
            fs::path full_path = fs::path(config.home) / _filename;
            _fptr.reset(std::fopen(full_path.string().c_str(), "ab+"));
            if (!_fptr) {
                throw std::runtime_error("Failed to open WAL file at: " + full_path.string());
            }
        }
        void pre_write(const Record& record) {}
        void post_write(const Record& record) {}

        private:
        std::unique_ptr<FILE, FileDeleter> _fptr;
    }
}
#endif