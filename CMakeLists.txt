cmake_minimum_required(VERSION 3.20)
project(TrabFinalED)

set(CMAKE_EXPORT_COMPILE_COMMADS ON)

if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug)
endif()

if (NOT MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG "-g")
   set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(SRC
   src/main.c
)

add_executable(proj ${SRC})
target_include_directories(proj PUBLIC src)

