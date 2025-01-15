cmake_minimum_required(VERSION 3.5.0)
project(webpier VERSION 1.0.0 LANGUAGES C CXX)

set(Boost_USE_STATIC_LIBS ON)
set(wxWidgets_USE_STATIC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    if(BUILD_TESTING)
        find_package(Boost REQUIRED COMPONENTS filesystem property_tree coroutine unit_test_framework)
    else()
        find_package(Boost REQUIRED COMPONENTS filesystem property_tree coroutine)
    endif()
    find_package(wxWidgets CONFIG REQUIRED core base propgrid)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -fPIC -fvisibility=hidden")
    if(BUILD_TESTING)
        find_package(Boost REQUIRED COMPONENTS filesystem coroutine unit_test_framework)
    else()
        find_package(Boost filesystem coroutine)
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

find_package(plexus REQUIRED)

message("*** plexus Version: ${plexus_VERSION}")
message("*** plexus Include: ${plexus_INCLUDE_DIR}")
message("*** plexus Library: ${plexus_LIB}")

find_package(wormhole REQUIRED)

message("*** wormhole Version: ${wormhole_VERSION}")
message("*** wormhole Include: ${wormhole_INCLUDE_DIR}")
message("*** wormhole Library: ${wormhole_LIB}")

find_package(tubus REQUIRED)

message("*** tubus Version: ${tubus_VERSION}")
message("*** tubus Include: ${tubus_INCLUDE_DIR}")
message("*** tubus Library: ${tubus_LIB}")

find_package(PkgConfig REQUIRED)

pkg_search_module(opendht REQUIRED IMPORTED_TARGET opendht>=3.2)
message("* opendht Include Dir: ${opendht_INCLUDEDIR}")
message("* opendht Lib Dir: ${opendht_LIBDIR}")
include_directories(${opendht_INCLUDE_DIR})
link_directories(${opendht_LIBDIR})

pkg_search_module(JsonCpp REQUIRED IMPORTED_TARGET jsoncpp)
message("* JsonCpp Include Dir: ${JsonCpp_INCLUDEDIR}")
message("* JsonCpp Lib Dir: ${JsonCpp_LIBDIR}")
include_directories(${JsonCpp_INCLUDE_DIR})
link_directories(${JsonCpp_LIBDIR})

pkg_search_module(libargon2 REQUIRED IMPORTED_TARGET libargon2)
message("* libargon2 Include Dir: ${libargon2_INCLUDEDIR}")
message("* libargon2 Lib Dir: ${libargon2_LIBDIR}")
include_directories(${libargon2_INCLUDE_DIR})
link_directories(${libargon2_LIBDIR})

pkg_search_module(fmt REQUIRED IMPORTED_TARGET fmt)
message("* fmt Include Dir: ${fmt_INCLUDEDIR}")
message("* fmt Lib Dir: ${fmt_LIBDIR}")
include_directories(${fmt_INCLUDE_DIR})
link_directories(${fmt_LIBDIR})

pkg_search_module(GnuTLS REQUIRED IMPORTED_TARGET gnutls)
message("* GnuTLS Include Dir: ${GnuTLS_INCLUDEDIR}")
message("* GnuTLS Lib Dir: ${GnuTLS_LIBDIR}")
include_directories(${GnuTLS_INCLUDE_DIR})
link_directories(${GnuTLS_LIBDIR})

pkg_search_module(nettle REQUIRED IMPORTED_TARGET nettle)
message("* nettle Include Dir: ${nettle_INCLUDEDIR}")
message("* nettle Lib Dir: ${nettle_LIBDIR}")
include_directories(${nettle_INCLUDE_DIR})
link_directories(${nettle_LIBDIR})

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
target_include_directories(${PROJECT_NAME} PRIVATE src ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} Boost::filesystem Boost::property_tree Boost::coroutine OpenSSL::SSL OpenSSL::Crypto Crypt32)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} Boost::filesystem Boost::coroutine OpenSSL::SSL OpenSSL::Crypto)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

set(SLIPWAY_MODULE slipway)
add_executable(${SLIPWAY_MODULE} WIN32 src/slipway/message.cpp src/slipway/server.cpp)
target_compile_definitions(${SLIPWAY_MODULE} PRIVATE PLEXUS_EXPORTS BOOST_BIND_GLOBAL_PLACEHOLDERS)
target_include_directories(${SLIPWAY_MODULE} PRIVATE src ${plexus_INCLUDE_DIR} ${wormhole_INCLUDE_DIR} ${tubus_INCLUDE_DIR})
if(WIN32)
    target_link_libraries(${SLIPWAY_MODULE} PRIVATE plexus wormhole tubus PkgConfig::opendht PkgConfig::fmt PkgConfig::libargon2 PkgConfig::GnuTLS PkgConfig::nettle PkgConfig::JsonCpp Boost::filesystem Boost::property_tree Boost::coroutine OpenSSL::SSL OpenSSL::Crypto Crypt32)
else()
    target_link_libraries(${SLIPWAY_MODULE} PRIVATE plexus wormhole tubus PkgConfig::opendht PkgConfig::fmt PkgConfig::libargon2 PkgConfig::GnuTLS PkgConfig::nettle PkgConfig::JsonCpp Boost::filesystem Boost::coroutine OpenSSL::SSL OpenSSL::Crypto)
endif()
set_target_properties(${SLIPWAY_MODULE} PROPERTIES DEBUG_POSTFIX "d")

if(BUILD_TESTING)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(SLIPWAY_EXEC "${SLIPWAY_MODULE}d")
    else()
        set(SLIPWAY_EXEC "${SLIPWAY_MODULE}")
    endif()
    set(WEBPIER_UT webpier_ut)
    add_executable(${WEBPIER_UT} src/store/tests/utils.cpp src/store/tests/context.cpp src/slipway/tests/message.cpp src/slipway/tests/slipway.cpp src/store/context.cpp src/slipway/message.cpp src/slipway/client.cpp src/store/utils.cpp)
    target_compile_definitions(${WEBPIER_UT} PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS SLIPWAY_EXEC="${CMAKE_CURRENT_BINARY_DIR}/${SLIPWAY_EXEC}")
    target_include_directories(${WEBPIER_UT} PRIVATE src ${Boost_INCLUDE_DIRS})
    if(WIN32)
        target_link_libraries(${WEBPIER_UT} PRIVATE Boost::filesystem Boost::property_tree Boost::unit_test_framework Boost::coroutine OpenSSL::SSL OpenSSL::Crypto Crypt32 ws2_32)
    else()
        target_link_libraries(${WEBPIER_UT} PRIVATE Boost::filesystem Boost::unit_test_framework Boost::coroutine OpenSSL::SSL OpenSSL::Crypto)
    endif()
    set_target_properties(${WEBPIER_UT} PROPERTIES DEBUG_POSTFIX "d")
    enable_testing()
    add_test(NAME ${WEBPIER_UT} COMMAND ${WEBPIER_UT} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
