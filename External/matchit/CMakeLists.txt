cmake_minimum_required(VERSION 3.0)
project(matchit)
set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/matchit/include
        )

file(GLOB Src
        "C:/3rdLibary/matchit/include/matchit.h"
        )

#message("matchit Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
