cmake_minimum_required(VERSION 3.10)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/subdir-header.cmake)
message(STATUS "Configuring CaLib...")

find_package(ZLIB REQUIRED)
find_package(Curses REQUIRED)

include_directories("include")
include_directories("${CMAKE_SOURCE_DIR}/Tools/a2display/inc")


set(MOD "CaLib")
ROOT_PREPARE_LIB("src" "include" "include" "src/G__${MOD}.cxx" SRCS EXESRCS)
add_library("lib${MOD}" SHARED ${SRCS})
target_link_libraries("lib${MOD}" ${ROOT_LIBRARIES} ${CURSES_LIBRARIES} ${ZLIB_LIBRARIES} Spectrum a2display)
set_target_properties("lib${MOD}" PROPERTIES OUTPUT_NAME ${MOD})

# ROOT 6 looks for the dictionary PCM next to the shared library when a macro
# loads libCaLib.so.  ROOT_GENERATE_DICTIONARY creates it in the source-specific
# build directory, so place a copy beside the library.
add_custom_command(TARGET "lib${MOD}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/src/G__${MOD}_rdict.pcm"
            "$<TARGET_FILE_DIR:lib${MOD}>/G__${MOD}_rdict.pcm")

# and we create the calib_manager
add_executable("calib_manager" "src/Main${MOD}Manager.cxx" ${ADDITIONAL_FILES})
target_link_libraries("calib_manager" "lib${MOD}")
