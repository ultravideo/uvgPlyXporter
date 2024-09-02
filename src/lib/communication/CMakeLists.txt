message(DEBUG "./src/lib/utils/cmake")

# Define the library
add_library(communicationLibrary STATIC
    portal.cpp
)


find_package(cppzmq CONFIG REQUIRED)

target_link_libraries(communicationLibrary
    cppzmq
    cppzmq-static
)

# # Optionally, add additional configurations, dependencies, or options
if(UNIX)
    target_compile_options(communicationLibrary PRIVATE -fPIC)
endif()