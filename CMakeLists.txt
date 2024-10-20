cmake_minimum_required(VERSION 3.5.0)
project(webpier VERSION 0.1.0 LANGUAGES C CXX)

set(Boost_USE_STATIC_LIBS ON)
set(wxWidgets_USE_STATIC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    if(BUILD_TESTING)
        find_package(Boost REQUIRED COMPONENTS property_tree unit_test_framework)
    else()
        find_package(Boost REQUIRED COMPONENTS property_tree)
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -fPIC -fvisibility=hidden")
    if(BUILD_TESTING)
        find_package(Boost REQUIRED COMPONENTS unit_test_framework)
    else()
        find_package(Boost)
    endif()
endif()

message("*** Boost Version: ${Boost_VERSION}")
message("*** Boost Include Dirs: ${Boost_INCLUDE_DIRS}")
message("*** Boost Librariy Dirs: ${Boost_LIBRARY_DIRS}")
message("*** Boost Libraries: ${Boost_LIBRARIES}")

find_package(wxWidgets REQUIRED core base propgrid)
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
    ui/main.cpp
    ui/mainframe.cpp
    ui/servicedialog.cpp
    ui/settingsdialog.cpp
    ui/importdialog.cpp
    ui/exportdialog.cpp
    ui/aboutdialog.cpp
    ui/messagedialog.cpp
    ui/startupdialog.cpp
    store/context.cpp
    store/utils.cpp
    )

target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS)
target_include_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

if(BUILD_TESTING)
    set(WEBPIER_UT webpier_ut)
    add_executable(${WEBPIER_UT} store/tests/utils.cpp store/tests/context.cpp store/context.cpp store/utils.cpp)
    target_compile_definitions(${WEBPIER_UT} PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS)
    target_include_directories(${WEBPIER_UT} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(${WEBPIER_UT} PRIVATE OpenSSL::SSL OpenSSL::Crypto Boost::unit_test_framework)
    set_target_properties(${WEBPIER_UT} PROPERTIES DEBUG_POSTFIX "d")

    enable_testing()
    add_test(NAME ${WEBPIER_UT} COMMAND ${WEBPIER_UT} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
