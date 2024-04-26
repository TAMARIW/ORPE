cmake_minimum_required(VERSION 3.5.0)
project(ORPE VERSION 0.1.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

#Collect and add source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

add_subdirectory(lib/ORPE)

add_executable(ESTIMATOR ${SRC_FILES} ipc/ipcORPE.cpp)

target_include_directories(ESTIMATOR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ipc/)
target_include_directories(ESTIMATOR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${LIBCAMERA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})

target_compile_options(ESTIMATOR PUBLIC -g)

target_link_libraries(${PROJECT_NAME} PUBLIC -llccv ORPELibrary)

#Add include directories
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

#Set C++ standard to 20	
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 