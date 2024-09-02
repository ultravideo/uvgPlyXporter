#include "plyxporter/plyxporter.hpp"
#include "communication/portal.hpp"

#include <unordered_map>

namespace plyxporter {
    
    namespace API {
        void test() {
            Portal portal;
            portal.zmq_run();
        }
    } // namespace API
} // namespace plyxporter
