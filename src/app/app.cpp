#include <iostream>
#include "plyxporter/plyxporter.hpp"

int main(int argc, char* argv[]) {
    std::string color_address = "";
    std::string position_address = "";
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--addr_color" && std::string(argv[i + 1]).substr(0, 1) != "-") {
                color_address = argv[i + 1];
        } else if (std::string(argv[i]) == "--addr_position" && std::string(argv[i + 1]).substr(0, 1) != "-") {
            position_address = argv[i + 1];
        }
    }

    if (color_address == "" || position_address == "") {
        std::cout << "Usage: " << argv[0] << " --addr_color <color_address> --addr_position <position_address>" << std::endl;
        return 1;
    }
    plyxporter::API::run(color_address, position_address);
    return 0;
}