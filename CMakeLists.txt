message("***** plexus ***** ")

########## setup ##########

cmake_minimum_required(VERSION 3.21)

project(webpier VERSION 1.0.0 DESCRIPTION "The program for forwarding TCP applications via NAT" LANGUAGES CXX)

option(WEBPIER_SKIP_TEST_RULES "Don't generate test rules" ON)
option(WEBPIER_SKIP_INSTALL_RULES "Don't generate install rules" OFF)
option(WEBPIER_SKIP_PACKAGE_RULES "Don't generate package rules" OFF)

########## dependencies ##########

if(MSVC)
    if(NOT WEBPIER_SKIP_TEST_RULES)
        find_package(Boost REQUIRED COMPONENTS filesystem process property_tree coroutine program_options unit_test_framework)
    else()
        find_package(Boost REQUIRED COMPONENTS filesystem process property_tree coroutine program_options)
    endif()
else()
    if(NOT WEBPIER_SKIP_TEST_RULES)
        find_package(Boost REQUIRED COMPONENTS filesystem coroutine program_options unit_test_framework)
    else()
        find_package(Boost filesystem coroutine program_options)
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

find_package(plexus REQUIRED)

message("*** plexus Version: ${plexus_VERSION}")
message("*** plexus Include: ${plexus_INCLUDE_DIRS}")
message("*** plexus Library: ${plexus_LIBRARY}")

find_package(wormhole REQUIRED)

message("*** wormhole Version: ${wormhole_VERSION}")
message("*** wormhole Include: ${wormhole_INCLUDE_DIRS}")
message("*** wormhole Library: ${wormhole_LIBRARY}")

find_package(tubus REQUIRED DEBUG)

message("*** tubus Version: ${tubus_VERSION}")
message("*** tubus Include: ${tubus_INCLUDE_DIRS}")
message("*** tubus Library: ${tubus_LIBRARY}")

find_package(PkgConfig REQUIRED)

pkg_search_module(opendht REQUIRED IMPORTED_TARGET opendht>=3.2)

message("* opendht Include Dir: ${opendht_INCLUDEDIR}")
message("* opendht Lib Dir: ${opendht_LIBDIR}")

pkg_search_module(nettle REQUIRED IMPORTED_TARGET nettle)

message("* nettle Include Dir: ${nettle_INCLUDEDIR}")
message("* nettle Lib Dir: ${nettle_LIBDIR}")

pkg_search_module(fmt REQUIRED IMPORTED_TARGET fmt)

message("* fmt Include Dir: ${fmt_INCLUDEDIR}")
message("* fmt Lib Dir: ${fmt_LIBDIR}")

pkg_search_module(libargon2 REQUIRED IMPORTED_TARGET libargon2)

message("* libargon2 Include Dir: ${libargon2_INCLUDEDIR}")
message("* libargon2 Lib Dir: ${libargon2_LIBDIR}")

########## build ##########

set(WEBPIER ${PROJECT_NAME})
set(SLIPWAY slipway)
set(CARRIER carrier)

add_executable(${WEBPIER} WIN32)
add_executable(${PROJECT_NAME}::${WEBPIER} ALIAS ${WEBPIER})

add_executable(${SLIPWAY})
add_executable(${PROJECT_NAME}::${SLIPWAY} ALIAS ${SLIPWAY})

add_executable(${CARRIER})
add_executable(${PROJECT_NAME}::${CARRIER} ALIAS ${CARRIER})

target_sources(${WEBPIER} PRIVATE     
    src/ui/main.cpp
    src/ui/mainframe.h
    src/ui/mainframe.cpp
    src/ui/servicedialog.h
    src/ui/servicedialog.cpp
    src/ui/settingsdialog.h
    src/ui/settingsdialog.cpp
    src/ui/exchangedialog.h
    src/ui/exchangedialog.cpp
    src/ui/aboutdialog.h
    src/ui/aboutdialog.cpp
    src/ui/messagedialog.h
    src/ui/messagedialog.cpp
    src/ui/startupdialog.h
    src/ui/startupdialog.cpp
    src/ui/context.h
    src/ui/context.cpp
    src/store/context.h
    src/store/context.cpp
    src/store/utils.h
    src/store/utils.cpp
    src/backend/message.h
    src/backend/message.cpp
    src/backend/client.h
    src/backend/client.cpp
    )

target_sources(${SLIPWAY} PRIVATE
    src/backend/message.h
    src/backend/message.cpp
    src/backend/server.cpp
    src/store/utils.h
    src/store/utils.cpp
    )

target_sources(${CARRIER} PRIVATE
    src/store/utils.h
    src/store/utils.cpp
    src/backend/tunnel.cpp
    )


target_link_libraries(${WEBPIER} PRIVATE ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${CARRIER} PRIVATE ${Boost_LIBRARIES} wormhole::libwormhole tubus::libtubus OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${SLIPWAY} PRIVATE ${Boost_LIBRARIES} plexus::libplexus wormhole::libwormhole tubus::libtubus PkgConfig::opendht PkgConfig::fmt PkgConfig::libargon2 PkgConfig::nettle OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
    target_link_libraries(${WEBPIER} PRIVATE Crypt32)
    target_link_libraries(${CARRIER} PRIVATE Crypt32)
    target_link_libraries(${SLIPWAY} PRIVATE Crypt32)
endif()

target_include_directories(${WEBPIER} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS})
target_include_directories(${SLIPWAY} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})
target_include_directories(${CARRIER} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})

set_target_properties(${WEBPIER} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${SLIPWAY} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${CARRIER} PROPERTIES DEBUG_POSTFIX "d")

set(WEBPIER_MODULE "$<IF:$<BOOL:${WEBPIER_MODULE}>,${WEBPIER_MODULE},${WEBPIER}>")
set(SLIPWAY_MODULE "$<IF:$<BOOL:${SLIPWAY_MODULE}>,${SLIPWAY_MODULE},${SLIPWAY}>")
set(CARRIER_MODULE "$<IF:$<BOOL:${CARRIER_MODULE}>,${CARRIER_MODULE},${CARRIER}>")

target_compile_definitions(${WEBPIER} PRIVATE WEBPIER_MODULE="${WEBPIER_MODULE}" SLIPWAY_MODULE="${SLIPWAY_MODULE}" CARRIER_MODULE="${CARRIER_MODULE}")
target_compile_definitions(${SLIPWAY} PRIVATE WEBPIER_MODULE="${WEBPIER_MODULE}" SLIPWAY_MODULE="${SLIPWAY_MODULE}" CARRIER_MODULE="${CARRIER_MODULE}")
target_compile_definitions(${CARRIER} PRIVATE WEBPIER_MODULE="${WEBPIER_MODULE}" SLIPWAY_MODULE="${SLIPWAY_MODULE}" CARRIER_MODULE="${CARRIER_MODULE}")

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

########## install ##########

if(NOT WEBPIER_SKIP_INSTALL_RULES AND NOT CMAKE_SKIP_INSTALL_RULES)

    include(GNUInstallDirs)

    if(MSVC)
        set(WEBPIER_INSTALL_BINDIR ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}/${CMAKE_INSTALL_BINDIR})
    else()
        if(CMAKE_INSTALL_PREFIX MATCHES "^(/usr)|(/usr/local)$")
            set(WEBPIER_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
        else()
            set(WEBPIER_INSTALL_BINDIR ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}/${CMAKE_INSTALL_BINDIR})
        endif()
    endif()

    install(TARGETS ${WEBPIER} EXPORT webpier_export RUNTIME DESTINATION ${WEBPIER_INSTALL_BINDIR} COMPONENT binary_files)
    install(TARGETS ${SLIPWAY} EXPORT webpier_export RUNTIME DESTINATION ${WEBPIER_INSTALL_BINDIR} COMPONENT binary_files)
    install(TARGETS ${CARRIER} EXPORT webpier_export RUNTIME DESTINATION ${WEBPIER_INSTALL_BINDIR} COMPONENT binary_files)

    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:${WEBPIER}>" COMPONENT pdb_files
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${WEBPIER_INSTALL_BINDIR}
            OPTIONAL)
        install(FILES "$<TARGET_PDB_FILE:${SLIPWAY}>" COMPONENT pdb_files
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${WEBPIER_INSTALL_BINDIR}
            OPTIONAL)
        install(FILES "$<TARGET_PDB_FILE:${CARRIER}>" COMPONENT pdb_files
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${WEBPIER_INSTALL_BINDIR}
            OPTIONAL)
    endif()
endif()

########## tests ##########

if(NOT WEBPIER_SKIP_INSTALL_RULES AND NOT CMAKE_SKIP_INSTALL_RULES)
    set(WEBPIER_TEST webpier_ut)
    add_executable(${WEBPIER_TEST} tests/utils.cpp tests/context.cpp tests/message.cpp tests/slipway.cpp src/store/context.cpp src/backend/message.cpp src/backend/client.cpp src/store/utils.cpp)
    target_link_libraries(${WEBPIER_TEST} PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
    if(WIN32)
        target_link_libraries(${WEBPIER_TEST} PRIVATE Crypt32 ws2_32)
    endif()
    target_compile_definitions(${WEBPIER_TEST} PRIVATE WEBPIER_MODULE="${WEBPIER_MODULE}" SLIPWAY_MODULE="${SLIPWAY_MODULE}" CARRIER_MODULE="${CARRIER_MODULE}")
    target_include_directories(${WEBPIER_TEST} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})
    set_target_properties(${WEBPIER_TEST} PROPERTIES DEBUG_POSTFIX "d")
    enable_testing()
    add_test(NAME ${WEBPIER_TEST} COMMAND ${WEBPIER_TEST} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

########## package ##########

if(NOT WEBPIER_SKIP_PACKAGE_RULES AND NOT WEBPIER_SKIP_INSTALL_RULES AND NOT CMAKE_SKIP_INSTALL_RULES)

    if(CMAKE_HOST_SYSTEM_MODULE MATCHES "Linux")
        cmake_host_system_information(RESULT _DISTRIB QUERY DISTRIB_ID)
        if(_DISTRIB MATCHES "(debian)|(ubuntu)|(kali)|(astra)")
            if(CMAKE_INSTALL_PREFIX MATCHES "^(/usr)|(/usr/local)$")
                set(_DOCDIR_PREFIX ${CMAKE_INSTALL_DOCDIR})
            else()
                set(_DOCDIR_PREFIX ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}/${CMAKE_INSTALL_DOCDIR})
            endif()

            add_custom_command(DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog"
                COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")
            add_custom_target(changelog ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

            set(_DEBIAN_DOC_FILES
                ${CMAKE_CURRENT_BINARY_DIR}/changelog.gz
                ${CMAKE_CURRENT_SOURCE_DIR}/debian/copyright
                ${CMAKE_CURRENT_SOURCE_DIR}/debian/README.Debian
                )
            install(FILES ${_DEBIAN_DOC_FILES} DESTINATION ${_DOCDIR_PREFIX}/${WEBPIER}-${${PROJECT_NAME}_VERSION} COMPONENT bin_doc_files)
        endif()
    endif()

    include(CPack)
endif()