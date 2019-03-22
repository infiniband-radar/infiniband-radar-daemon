cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

set(TARGET_NAME infiniband_radar_daemon)
project(${TARGET_NAME})

include_directories(${CMAKE_SOURCE_DIR}/3d_party/nlohmann__json/include/)
message(${CMAKE_SOURCE_DIR}/3d_party/nlohmann__json/include/)

add_executable(${TARGET_NAME} src/main.cpp src/HumanReadable.h src/IBUtils.h src/IBIncludes.h src/FabricDiscoveryHelper.h src/FabricDiscoveryHelper.cpp src/InfiniBandRadar.cpp src/InfiniBandRadar.h src/ApiClient.cpp src/ApiClient.h src/IBUtils.cpp)
include_directories("/usr/include/infiniband/")
target_link_libraries(${PROJECT_NAME} ibnetdisc ibmad curl pthread)
