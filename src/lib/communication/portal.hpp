#pragma once

#ifndef PORTAL_HPP
#define PORTAL_HPP

#include <zmq.hpp>
#include <thread>  
#include <queue>
#include <vector>

#include "plyxporter/threadqueue.hpp"
#include "plyxporter/log.hpp"

#include <mutex>
#include <condition_variable>

#include "tinyply.h"

namespace plyxporter {
    typedef struct Vec3 {
        float x;
        float y;
        float z;
    } Vec3;

    typedef struct VecU8 {
        uint8_t x;
        uint8_t y;
        uint8_t z;
    } VecU8;

    struct zmqHandler {
        // Communication
        std::string color_address = "tcp://*:5555";             // Default address for color
        std::string position_address = "tcp://*:5556";          // Default address for position
        std::queue<zmq::message_t> colorMessages;               // Queue for color messages
        std::queue<zmq::message_t> positionMessages;            // Queue for position messages
        std::condition_variable receive_message_cv;             // Condition variable for receiving messages
    };

    class Portal { 
    private:
        /*** ZMQ ***/  
        std::shared_ptr<zmqHandler> zmq_handler = nullptr;       // ZMQ Handler
        /***** *****/
        std::mutex receive_message_mutex;
        std::condition_variable receive_message_cv;
        int export_count = 0;

        /*** Control & Data ***/ 
        bool stop_flag = false;
        /*** ************** ***/ 
    public:
        Portal();
        ~Portal();

        void set_position_socket(std::string address);
        void set_color_socket(std::string address);
        
        void stop_signal();

        void set_sequence_path(std::string folder_path);

        void zmq_run();

    private:
        void export_ply(std::string filename, std::shared_ptr<std::vector<Vec3>> &vertices, std::shared_ptr<std::vector<VecU8>> &colors);
    };
};

#endif // PORTAL_HPP