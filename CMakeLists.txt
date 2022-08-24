cmake_minimum_required(VERSION 3.1)
project(UHDTestApp)

set(PROJECT_VERSION 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(UHD REQUIRED)
find_package(Boost REQUIRED)

add_executable(uhd-test-app 
    main.cpp
    src/Radio.cpp
    src/DataSource.cpp)

target_include_directories(uhd-test-app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(uhd-test-app
            boost_program_options
            boost_thread
            uhd
            pthread
            )