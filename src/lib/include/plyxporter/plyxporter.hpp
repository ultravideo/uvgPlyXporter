#pragma once
#ifndef PLYEXPORTER_HPP
#define PLYEXPORTER_HPP

#include "log.hpp"
#include "threadqueue.hpp"

namespace plyxporter {
    
    namespace API {
        void run(std::string _color_addr, std::string _position_addr);
    } // namespace API
} 

#endif // PLYEXPORTER_HPP