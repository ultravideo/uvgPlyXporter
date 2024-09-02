message(DEBUG "./src/app/cmake")
include_directories(${CMAKE_SOURCE_DIR}/src/lib ${CMAKE_SOURCE_DIR}/src/lib/include)

# Define the executable
add_executable(plyXporter
    app.cpp
    # cli.cpp
)

set(CMAKE_CXX_CLANG_TIDY "")

target_link_libraries(plyXporter plyXporter_API)

# Define installation destinations
# install(TARGETS plyXporter DESTINATION bin)