cmake_minimum_required(VERSION 3.5.0)
project(webpier VERSION 1.0.0 LANGUAGES C CXX)

set(Boost_USE_STATIC_LIBS ON)
set(wxWidgets_USE_STATIC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    if(BUILD_TESTING)
        find_package(Boost REQUIRED COMPONENTS property_tree coroutine unit_test_framework)
    else()
        find_package(Boost REQUIRED COMPONENTS property_tree coroutine)
    endif()
    find_package(wxWidgets CONFIG REQUIRED core base propgrid)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -fPIC -fvisibility=hidden")
    if(BUILD_TESTING)
        find_package(Boost REQUIRED COMPONENTS coroutine unit_test_framework)
    else()
        find_package(Boost coroutine)
    endif()
    find_package(wxWidgets REQUIRED core base propgrid)
endif()

message("*** Boost Version: ${Boost_VERSION}")
message("*** Boost Include Dirs: ${Boost_INCLUDE_DIRS}")
message("*** Boost Librariy Dirs: ${Boost_LIBRARY_DIRS}")
message("*** Boost Libraries: ${Boost_LIBRARIES}")

if(wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif()

message("*** wxWidgets Includes: ${wxWidgets_INCLUDE_DIRS}")
message("*** wxWidgets Libraries: ${wxWidgets_LIBRARIES}")

find_package(OpenSSL REQUIRED)

message("*** OpenSSL Version: ${OPENSSL_VERSION}")
message("*** OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
message("*** OpenSSL Libraries: ${OPENSSL_LIBRARIES}")

add_executable(${PROJECT_NAME} WIN32 
    src/ui/main.cpp
    src/ui/mainframe.cpp
    src/ui/servicedialog.cpp
    src/ui/settingsdialog.cpp
    src/ui/exchangedialog.cpp
    src/ui/aboutdialog.cpp
    src/ui/messagedialog.cpp
    src/ui/startupdialog.cpp
    src/ui/context.cpp
    src/store/context.cpp
    src/store/utils.cpp
    src/slipway/message.cpp
    src/slipway/client.cpp
    )

target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS)
target_include_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} Boost::property_tree Boost::coroutine OpenSSL::SSL OpenSSL::Crypto Crypt32)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} Boost::coroutine OpenSSL::SSL OpenSSL::Crypto)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

set(SLIPWAY_MODULE slipway)
add_executable(${SLIPWAY_MODULE} WIN32 src/slipway/message.cpp src/slipway/server.cpp)
if(WIN32)
    target_link_libraries(${SLIPWAY_MODULE} PRIVATE ${wxWidgets_LIBRARIES} Boost::property_tree Boost::coroutine)
else()
    target_link_libraries(${SLIPWAY_MODULE} PRIVATE ${wxWidgets_LIBRARIES} Boost::coroutine)
endif()
set_target_properties(${SLIPWAY_MODULE} PROPERTIES DEBUG_POSTFIX "d")

if(BUILD_TESTING)
    set(WEBPIER_UT webpier_ut)
    add_executable(${WEBPIER_UT} src/store/tests/utils.cpp src/store/tests/context.cpp src/slipway/tests/message.cpp src/store/context.cpp src/slipway/message.cpp src/store/utils.cpp)
    target_compile_definitions(${WEBPIER_UT} PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS)
    target_include_directories(${WEBPIER_UT} PRIVATE ${Boost_INCLUDE_DIRS})
    if(WIN32)
        target_link_libraries(${WEBPIER_UT} PRIVATE Boost::property_tree Boost::unit_test_framework Boost::coroutine OpenSSL::SSL OpenSSL::Crypto Crypt32 ws2_32)
    else()
        target_link_libraries(${WEBPIER_UT} PRIVATE Boost::unit_test_framework Boost::coroutine OpenSSL::SSL OpenSSL::Crypto)
    endif()
    set_target_properties(${WEBPIER_UT} PROPERTIES DEBUG_POSTFIX "d")
    enable_testing()
    add_test(NAME ${WEBPIER_UT} COMMAND ${WEBPIER_UT} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
