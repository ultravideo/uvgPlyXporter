#include "plyxporter/plyxporter.hpp"
#include "communication/portal.hpp"

#include <unordered_map>

namespace plyxporter {
    namespace API {
        void run(std::string _color_addr, std::string _position_addr) {
            Portal portal(_color_addr, _position_addr);
            portal.zmq_run();
        }
    } // namespace API
} // namespace plyxporter
