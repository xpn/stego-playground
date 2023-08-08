cmake_minimum_required(VERSION 3.15)

project(png_stego
    VERSION 0.1
    DESCRIPTION "PNG Stego POC"
    LANGUAGES CXX)

include_directories(include)

add_executable(png_encoder 
    src/encoder.cpp
    lib/bititerator.cpp
    lib/compression.cpp
    lib/pngstego.cpp
)

add_executable(png_decoder 
    src/decoder.cpp
    lib/bititerator.cpp
    lib/compression.cpp
    lib/pngstego.cpp
)

target_link_libraries(png_encoder PRIVATE z)
target_link_libraries(png_decoder PRIVATE z)

target_compile_features(png_encoder PRIVATE cxx_std_17)
target_compile_features(png_decoder PRIVATE cxx_std_17)
