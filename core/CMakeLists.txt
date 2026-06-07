cmake_minimum_required(VERSION 3.20)
project(socium_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Opciones de paralelización (HPC)
option(SOCIUM_ENABLE_OPENMP "Build with OpenMP" ON)
option(SOCIUM_ENABLE_MPI    "Build with MPI"    OFF)
option(SOCIUM_NATIVE        "Optimizar para la CPU local (-march=native)" OFF)

# --- librería del núcleo ---
add_library(socium_core
  src/economy/economy.cpp
  src/agents/population.cpp
)
target_include_directories(socium_core PUBLIC include)
target_compile_options(socium_core PRIVATE -O3 -Wall -Wextra)
if(SOCIUM_NATIVE)
  target_compile_options(socium_core PRIVATE -march=native)
endif()

if(SOCIUM_ENABLE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    target_link_libraries(socium_core PUBLIC OpenMP::OpenMP_CXX)
  endif()
endif()

if(SOCIUM_ENABLE_MPI)
  find_package(MPI)
  if(MPI_CXX_FOUND)
    target_link_libraries(socium_core PUBLIC MPI::MPI_CXX)
  endif()
endif()

# --- ejecutables ---
add_executable(socium_kwem apps/kinetic_exchange.cpp)
target_link_libraries(socium_kwem PRIVATE socium_core)

add_executable(socium_poblacion apps/population_demo.cpp)
target_link_libraries(socium_poblacion PRIVATE socium_core)

enable_testing()
# add_subdirectory(tests)
