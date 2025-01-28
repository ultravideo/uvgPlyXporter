
#include "portal.hpp"
#include <filesystem>
#include <fstream>



namespace plyxporter {
    Portal::Portal(std::string _color_addr, std::string _position_addr, std::string _save_directory) {
        zmq_handler = std::make_shared<zmqHandler>();
        zmq_handler->color_address = _color_addr;
        zmq_handler->position_address = _position_addr;
        zmq_handler->save_directory = _save_directory;


        if (!std::filesystem::exists(zmq_handler->save_directory)) {
            std::filesystem::create_directories(zmq_handler->save_directory);
        }
    }

    Portal::~Portal() {
    }

    void Portal::set_position_socket(std::string address) {
        zmq_handler->position_address = address;
    }

    void Portal::set_color_socket(std::string address) {
        zmq_handler->color_address = address;
    }

    void Portal::stop_signal() {
        stop_flag = true;
        zmq_handler->receive_message_cv.notify_one();
    }

    void Portal::set_sequence_path(std::string folder_path) {

    }

    void Portal::zmq_run() {

        std::cout << "Color Address: " << zmq_handler->color_address << std::endl;
        std::cout << "Position Address: " << zmq_handler->position_address << std::endl;
        stop_flag = false;
        zmq::context_t context{1};
        zmq::socket_t colorSocket(context, ZMQ_PULL);
        zmq::socket_t positionSocket(context, ZMQ_PULL);
        colorSocket.bind(zmq_handler->color_address);
        positionSocket.bind(zmq_handler->position_address);

        std::mutex receive_message_mutex;

        Logger::log(LogLevel::INFO, "Portal", "Context created\n");

        //2 thread for receiving color and position messages
        std::thread colorProcessThread([this, &colorSocket]() {
            Logger::log(LogLevel::INFO, "Portal", "Color thread started\n");
            while (!stop_flag) {
                // Receive the color message from the client
                zmq::message_t colorMessage;
                auto colorRes = colorSocket.recv(colorMessage, zmq::recv_flags::none);

                if (!colorRes.has_value()) { continue; }

                //if the message is "DISCONNECT", stop the loop
                if (colorMessage.to_string() == "DISCONNECT") {
                    stop_signal();
                    break;
                }

                zmq_handler->colorMessages.push(std::move(colorMessage));
                zmq_handler->receive_message_cv.notify_one();
            }
            Logger::log(LogLevel::INFO, "Portal", "Color thread stopped\n");
        });

        std::thread positionProcessThread([this, &positionSocket]() {
            Logger::log(LogLevel::INFO, "Portal", "Position thread started\n");
            
            while (!stop_flag) {
                // Receive the position message from the client
                zmq::message_t positionMessage;
                auto positionRes = positionSocket.recv(positionMessage, zmq::recv_flags::none);
                if (!positionRes.has_value()) { continue; }

                 //if the message is "DISCONNECT", stop the loop
                if (positionMessage.to_string() == "DISCONNECT") {
                    stop_signal();
                    break;
                }
   
                zmq_handler->positionMessages.push(std::move(positionMessage));
                zmq_handler->receive_message_cv.notify_one();
            }
            Logger::log(LogLevel::INFO, "Portal", "Position thread stopped\n");
        }); 


        std::unique_lock<std::mutex> lck(receive_message_mutex);
        std::shared_ptr<ThreadQueue> thread_queue = std::make_shared<ThreadQueue>(40);
        int count = 0;
        
        while (!stop_flag) {
            zmq_handler->receive_message_cv.wait(lck, [this] { return (!zmq_handler->colorMessages.empty() && !zmq_handler->positionMessages.empty()) || stop_flag; });

            if (stop_flag) {
                break;
            }

            auto Pmessage = std::move(zmq_handler->positionMessages.front());
            auto Cmessage = std::move(zmq_handler->colorMessages.front());

#ifndef POINT_UINT16
            size_t numPoints = Pmessage.size() / (sizeof(Vec3));
            std::shared_ptr<std::vector<Vec3>> positions = std::make_shared<std::vector<Vec3>>(reinterpret_cast<const Vec3*>(Pmessage.data()), reinterpret_cast<const Vec3*>(Pmessage.data()) + numPoints);
#else
            size_t numPoints = Pmessage.size() / (sizeof(VecU16));
            std::shared_ptr<std::vector<VecU16>> positions = std::make_shared<std::vector<VecU16>>(reinterpret_cast<const VecU16*>(Pmessage.data()), reinterpret_cast<const VecU16*>(Pmessage.data()) + numPoints);
#endif
            std::shared_ptr<std::vector<VecU8>> attributes = std::make_shared<std::vector<VecU8>>(reinterpret_cast<const VecU8*>(Cmessage.data()), reinterpret_cast<const VecU8*>(Cmessage.data()) + numPoints);
            count++;

            // //print 1st 5 points
            // for (int i = 0; i < 5; i++) {
            //     std::cout << "Position: " << positions->at(i).x << " " << positions->at(i).y << " " << positions->at(i).z << std::endl;
            //     std::cout << "Color: " << (int)attributes->at(i).x << " " << (int)attributes->at(i).y << " " << (int)attributes->at(i).z << std::endl;
            // }

            // if count < 10 => count_str = 0001, 0002, 0003, ...
            // if count < 100 => count_str = 0010, 0011, 0012, ...
            // if count < 1000 => count_str = 0100, 0101, 0102, ...
            std::string count_str = std::to_string(count);
            while (count_str.size() < 4) {
                count_str = "0" + count_str;
            }
            
            std::string filename = zmq_handler->save_directory + "/binary-";
            std::shared_ptr<Job> export_job = std::make_shared<Job>("ExportJob", 0, &Portal::export_ply, this, filename + count_str, positions, attributes);

            // export_ply("C:/Users/Guillaume/workspace/plyXporter/build/bin/PLY/test" + std::to_string(count), positions, attributes);

            thread_queue->submitJob(export_job);

            zmq_handler->positionMessages.pop();
            zmq_handler->colorMessages.pop();
        }

        Logger::log(LogLevel::INFO, "Portal", "Receiving total of " + std::to_string(count) + " frames\n");
        Logger::log(LogLevel::INFO, "Portal", "Stopping Components Threads\n");

        // Sleep for 5 seconds to allow the components to stop
        std::this_thread::sleep_for(std::chrono::seconds(5));

        colorProcessThread.join();
        positionProcessThread.join();

        colorSocket.close();
        positionSocket.close();
        context.close();

        Logger::log(LogLevel::INFO, "Portal", "Context closed\n");

        receive_message_cv.wait(lck, [this, count] { return count == export_count; });
        Logger::log(LogLevel::INFO, "Portal", "Exported " + std::to_string(export_count) + " frames\n");
    }

#ifndef POINT_UINT16
    void Portal::export_ply(std::string filename, std::shared_ptr<std::vector<Vec3>> &vertices, std::shared_ptr<std::vector<VecU8>> &colors) {
#else
    void Portal::export_ply(std::string filename, std::shared_ptr<std::vector<VecU16>> &vertices, std::shared_ptr<std::vector<VecU8>> &colors) {
#endif
        std::filebuf fb_binary;
        fb_binary.open(filename + ".ply", std::ios::out | std::ios::binary);
        std::ostream outstream_binary(&fb_binary);
        if (outstream_binary.fail()) throw std::runtime_error("failed to open " + filename);

        tinyply::PlyFile plyFile;

#ifdef POINT_UINT16
        std::vector<Vec3> verts(vertices->size());
        // transform vertices to float
        for (size_t i = 0; i < vertices->size(); i++) {
            // std::cout << vertices->at(i).x << " " << vertices->at(i).y << " " << vertices->at(i).z << std::endl;
            verts[i].x = static_cast<float>(vertices->at(i).x);
            verts[i].y = static_cast<float>(vertices->at(i).y);
            verts[i].z = static_cast<float>(vertices->at(i).z);
            // std:: cout << verts[i].x << " " << verts[i].y << " " << verts[i].z << std::endl;
        }
        plyFile.add_properties_to_element("vertex", { "x", "y", "z" }, tinyply::Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>(verts.data()), tinyply::Type::INVALID, 0);
#else
        plyFile.add_properties_to_element("vertex", { "x", "y", "z" }, tinyply::Type::FLOAT32, vertices->size(), reinterpret_cast<uint8_t*>(vertices->data()), tinyply::Type::INVALID, 0);
#endif
        // plyFile.add_properties_to_element("vertex", { "x", "y", "z" }, tinyply::Type::UINT16, vertices->size(), reinterpret_cast<uint8_t*>(vertices->data()), tinyply::Type::INVALID, 0);
        plyFile.add_properties_to_element("vertex", { "red", "green", "blue" }, tinyply::Type::UINT8, colors->size(), reinterpret_cast<uint8_t*>(colors->data()), tinyply::Type::INVALID, 0);
        plyFile.get_comments().push_back("generated by tinyply 2.3");

        plyFile.write(outstream_binary, true);

        std::lock_guard<std::mutex> lock(receive_message_mutex);
        export_count++;

        receive_message_cv.notify_one();
    }
}; // namespace plyxporter
