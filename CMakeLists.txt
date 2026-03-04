cmake_minimum_required(VERSION 3.15)

project(IonoMoni)

add_definitions(-D_HAS_STD_BYTE=0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Header file paths
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/src
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gall
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gcoders
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gdata
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gexport
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gio
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gmodels
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gproc
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gprod
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gset
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/gutils
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/newmat
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/pugixml/src
    ${CMAKE_SOURCE_DIR}/include/src/LibGnut/spdlog
    ${CMAKE_SOURCE_DIR}/include/src/LibGREAT
    ${CMAKE_SOURCE_DIR}/include/src/LibGREAT/gcoders
    ${CMAKE_SOURCE_DIR}/include/src/LibGREAT/gdata
    ${CMAKE_SOURCE_DIR}/include/src/LibGREAT/gutils
    ${CMAKE_SOURCE_DIR}/include/src/third-party
    ${CMAKE_SOURCE_DIR}/include/src/third-party/Eigen
)

# Library paths
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Automatically collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/IonoMoni/*.h")

# Executable program
add_executable(IonoMoni
    src/IonoMoni.cpp
    src/gcfg_ppp.cpp   
 "src/extract_obs.cpp" "include/extract_obs.h" "include/decode_obs.h" "src/decode_obs.cpp"    "src/calc_elevation.cpp" "src/extract_arcs.cpp"  "src/read_sp3.cpp" "src/LagrangeInterpolation.cpp"   "src/obs.cpp" "src/extract_TEC.cpp" "src/read_Time_sp3.cpp" "src/read_DCB.cpp" "src/sp3.cpp" "src/stringToDouble.cpp"  "src/ecef2elli.cpp"  "include/constants.h"     "include/calc_elevation.h" "include/extract_arcs.h"  "include/read_sp3.h"  "include/LagrangeInterpolation.h"   "include/obs.h" "include/extract_TEC.h" "include/read_Time_sp3.h" "include/read_DCB.h" "include/sp3.h" "include/stringToDouble.h"  "include/ecef2elli.h"  "src/calc_AATR.cpp"  "include/calc_AATR.h" "src/calc_ROTI.cpp"  "include/calc_ROTI.h"  "src/process_sp3.cpp" "include/process_sp3.h" "src/get_dcb_path.cpp" "include/get_dcb_path.h" "src/get_sp3_paths.cpp" "include/get_sp3_paths.h"     "src/detect_cycle_slip.cpp"  "include/detect_cycle_slip.h" "src/calc_ipp.cpp" "include/calc_ipp.h" "src/mapping_function.cpp" "include/mapping_function.h" "src/read_DCB_1.0.cpp" "include/read_DCB_1.0.h" "src/ppp+.cpp" "include/ppp+.h")
target_link_options(IonoMoni PRIVATE "/STACK:134217728")

# Link libraries
target_link_libraries(IonoMoni
    LibGREAT
    LibGnut
)


