cmake_minimum_required(VERSION 3.16)
project(extract_data)

set(CMAKE_CXX_STANDARD 17)

# Enable vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Admin/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

find_package(OpenCV REQUIRED)
find_package(ZXing REQUIRED)

include_directories(include)
# Add all source files
add_executable(extract_data
    src/main.cpp
    src/audio_decoder.cpp
    src/qr_decoder.cpp
    src/kiss_fft.c
)

target_include_directories(extract_data PRIVATE include)
target_link_libraries(extract_data PRIVATE ${OpenCV_LIBS} ZXing::ZXing)

add_library(video_decoder SHARED
    src/interface.cpp
    src/audio_decoder.cpp
    src/qr_decoder.cpp
    src/kiss_fft.c
)
target_link_libraries(video_decoder PRIVATE ${OpenCV_LIBS} ZXing::ZXing)