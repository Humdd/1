cmake_minimum_required(VERSION 3.0)
project(iteration_utils)
set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/iteration_utils
        )

file(GLOB Src
        "C:/3rdLibary/iteration_utils/range.h"
        "C:/3rdLibary/iteration_utils/enumerate.h"
        )

#message("iteration_utils Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
