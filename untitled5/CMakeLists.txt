cmake_minimum_required(VERSION 3.26)
project(mountain_solver C)

set(CMAKE_C_STANDARD 11)

add_executable(mountain_solver
        gradient.c
        gradient_sol.c
)

target_link_libraries(mountain_solver m)