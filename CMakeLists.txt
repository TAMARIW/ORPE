cmake_minimum_required(VERSION 3.5.0)
project(ORPE VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
set (CMAKE_MODULE_PATH "/usr/local/lib/cmake/${CMAKE_MODULE_PATH}")
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)


#Collect and add source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

add_subdirectory(lib/ORPE)
add_subdirectory(lib/udpipc)

#add_executable(ESTIMATOR ${SRC_FILES} ipc/ipcORPE.cpp)

#target_include_directories(ESTIMATOR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ipc/)
#target_include_directories(ESTIMATOR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
#include_directories(${LIBCAMERA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})

#target_compile_options(${PROJECT_NAME} PUBLIC -g)

include_directories(${LIBCAMERA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC -llccv ORPELibrary udpipc ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})

#Add include directories
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
