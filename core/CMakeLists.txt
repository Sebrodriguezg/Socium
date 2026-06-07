cmake_minimum_required(VERSION 3.20)
project(socium_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opciones de paralelización (HPC)
option(SOCIUM_ENABLE_OPENMP "Build with OpenMP" ON)
option(SOCIUM_ENABLE_MPI    "Build with MPI"    OFF)

# add_library(socium_core ...)  # TODO: agregar fuentes en src/
# target_include_directories(socium_core PUBLIC include)

if(SOCIUM_ENABLE_OPENMP)
  find_package(OpenMP)
endif()

if(SOCIUM_ENABLE_MPI)
  find_package(MPI)
endif()

enable_testing()
# add_subdirectory(tests)
