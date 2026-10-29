cmake_minimum_required(VERSION 3.15)

project(VolUX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_DIR "C:/SFML/lib/cmake/SFML")
find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

add_executable(VolUX main.cpp)

target_link_libraries(VolUX 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
)

if(WIN32)
    add_custom_command(TARGET VolUX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:VolUX>
        $<TARGET_FILE_DIR:VolUX>
        COMMAND_EXPAND_LISTS)
endif()