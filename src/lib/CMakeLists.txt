message(DEBUG "./src/lib/cmake")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_file(version.cpp.in version.cpp)

add_subdirectory(utils)
add_subdirectory(communication)

set(LIB_SOURCES
    version.cpp
    plyxporter.cpp
)

if(BUILD_SHARED_LIBS)
  add_library(plyXporter_API SHARED ${LIB_SOURCES})
else()
  add_library(plyXporter_API STATIC ${LIB_SOURCES})
endif()

find_package(tinyply CONFIG REQUIRED)

target_link_libraries(plyXporter_API
    utilsLibrary
    logLibrary
    communicationLibrary
    tinyply
)

# Include directories for headers
target_include_directories(plyXporter_API PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optionally, add additional configurations, dependencies, or options
# target_compile_options(BitstreamGenerationLibrary PRIVATE -std=c++11)

# Define installation destinations
# install(TARGETS plyXporter DESTINATION lib)
# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include)