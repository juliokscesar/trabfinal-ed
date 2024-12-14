cmake_minimum_required(VERSION 3.20)
project(TrabFinalED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug)
endif()

if (NOT MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG "-g")
   set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/)

set(SRC
        src/main.c
        src/markov.c
        src/utils.c
        src/logging.c
        src/markovgraph.c
        src/markovnetwork.c
)

set(HEADER
        src/markov.h
        src/typedefs.h
        src/utils.h
        src/logging.h
        src/markovgraph.h
        src/markovnetwork.h
)

add_executable(proj ${SRC} ${HEADER})

target_link_libraries(proj PUBLIC
   -lm
)
