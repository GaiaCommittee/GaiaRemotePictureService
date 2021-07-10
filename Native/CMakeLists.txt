#==============================
# Requirements
#==============================

cmake_minimum_required(VERSION 3.10)

#==============================
# Project Settings
#==============================

if (NOT PROJECT_DECLARED)
    project("Gaia Remote Picture Service" LANGUAGES CXX VERSION 0.8)
    set(PROJECT_DECLARED)
endif()

if (NOT PROJECT_SUIT)
    set(PROJECT_SUIT TRUE)
endif()

#==============================
# Project Modules
#==============================

add_subdirectory("GaiaRemotePictureServer")
add_subdirectory("GaiaRemotePictureClient")
add_subdirectory("GaiaRemotePictureViewer")

if (WITH_TEST)
endif()
