cmake_minimum_required(VERSION 3.5)

project(HeLeCheLun LANGUAGES CXX)

include(CheckCXXCompilerFlag)
include(ExternalProject)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEPENDENCIES_INSTALL_DIR ${CMAKE_BINARY_DIR}/output-deps)

include(cmake/build_props.cmake)
include(cmake/library_FreeImage.cmake)
include(ext/Utf8Main/utf8main.cmake)

if(NOT WIN32 AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX))
    check_cxx_compiler_flag("-no-pie" HAS_NO_PIE)
endif()

function(pge_set_nopie _target)
    set_target_properties(${_target} PROPERTIES
        POSITION_INDEPENDENT_CODE False
    )
    if(HAS_NO_PIE AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX))
        set_property(TARGET ${_target} APPEND_STRING PROPERTY LINK_FLAGS " -no-pie")
    endif()
endfunction()

add_executable(HeLeCheLun main.cpp ${UTF8MAIN_SRCS})
add_dependencies(HeLeCheLun FreeImage_Local)
pge_set_nopie(HeLeCheLun)
target_include_directories(HeLeCheLun PRIVATE
    "${DEPENDENCIES_INSTALL_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext"
)
target_link_libraries(HeLeCheLun PRIVATE PGE_FreeImage)

if(NOT APPLE AND NOT MSVC)
    set_property(TARGET HeLeCheLun APPEND_STRING PROPERTY LINK_FLAGS " -static-libgcc -static-libstdc++ -static")
endif()

if(WIN32)
    set_target_properties(HeLeCheLun PROPERTIES WIN32_EXECUTABLE OFF)
    target_compile_definitions(HeLeCheLun PRIVATE -DNOMINMAX)
endif()
