cmake_minimum_required(VERSION 3.4...3.18)
project(pySxaudio)

add_subdirectory(pybind11)
pybind11_add_module(pySxaudio src/main.cpp src/wavfile.cpp src/spectrum.cpp src/sxmath.cpp)
