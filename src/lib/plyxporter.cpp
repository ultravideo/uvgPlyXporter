#include "plyxporter/plyxporter.hpp"
#include "communication/portal.hpp"

#include <unordered_map>

namespace plyxporter {
    namespace API {
        void run(std::string _color_addr, std::string _position_addr, std::string _save_directory) {
            Portal portal(_color_addr, _position_addr, _save_directory);
            portal.zmq_run();
        }
    } // namespace API
} // namespace plyxporter
