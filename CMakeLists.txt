message("***** webpier ***** ")

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
    if (NOT BUILD_SHARED_LIBS)
        set(Boost_USE_STATIC_LIBS ON)
    endif()
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

find_package(wxWidgets COMPONENTS core base propgrid QUIET)

if (NOT wxWidgets_FOUND)
    find_package(wxWidgets CONFIG REQUIRED core base propgrid)
endif()

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

find_package(opendht 3.2 REQUIRED)
find_package(fmt REQUIRED)
find_package(msgpack-cxx REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(GnuTLS REQUIRED IMPORTED_TARGET gnutls)

message("* GnuTLS Include Dir: ${GnuTLS_INCLUDEDIR}")
message("* GnuTLS Lib Dir: ${GnuTLS_LIBDIR}")

pkg_search_module(Nettle REQUIRED IMPORTED_TARGET nettle)

message("* Nettle Include Dir: ${Nettle_INCLUDEDIR}")
message("* Nettle Lib Dir: ${Nettle_LIBDIR}")

pkg_search_module(argon2 REQUIRED IMPORTED_TARGET libargon2)

message("* argon2 Include Dir: ${argon2_INCLUDEDIR}")
message("* argon2 Lib Dir: ${argon2_LIBDIR}")

pkg_search_module(Jsoncpp REQUIRED IMPORTED_TARGET jsoncpp)

message("* Jsoncpp Include Dir: ${Jsoncpp_INCLUDEDIR}")
message("* Jsoncpp Lib Dir: ${Jsoncpp_LIBDIR}")

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

include(cmake/utils.cmake)

if(NOT WIN32)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/webpier.ico ${CMAKE_CURRENT_BINARY_DIR}/assets/logo.h webpier_logo_ico)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/blue.png ${CMAKE_CURRENT_BINARY_DIR}/assets/blue.h blue_circle_png)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/red.png ${CMAKE_CURRENT_BINARY_DIR}/assets/red.h red_circle_png)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/green.png ${CMAKE_CURRENT_BINARY_DIR}/assets/green.h green_circle_png)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/grey.png ${CMAKE_CURRENT_BINARY_DIR}/assets/grey.h grey_circle_png)
endif()

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
    "$<$<BOOL:${WIN32}>:windows/webpier.rc>"
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

target_link_libraries(${WEBPIER} PRIVATE ${wxWidgets_LIBRARIES} Boost::coroutine Boost::filesystem Boost::program_options "$<$<BOOL:${MSVC}>:Boost::property_tree>" "$<$<BOOL:${MSVC}>:Crypt32>" OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${CARRIER} PRIVATE Boost::filesystem Boost::program_options "$<$<BOOL:${MSVC}>:Crypt32>" wormhole::libwormhole tubus::libtubus OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${SLIPWAY} PRIVATE Boost::coroutine Boost::filesystem Boost::program_options "$<$<BOOL:${MSVC}>:Boost::property_tree>" "$<$<BOOL:${MSVC}>:Crypt32>" plexus::libplexus wormhole::libwormhole tubus::libtubus opendht fmt::fmt msgpack-cxx PkgConfig::GnuTLS PkgConfig::argon2 PkgConfig::Nettle PkgConfig::Jsoncpp OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${WEBPIER} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS})
target_include_directories(${SLIPWAY} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})
target_include_directories(${CARRIER} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})

set_target_properties(${WEBPIER} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${SLIPWAY} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${CARRIER} PROPERTIES DEBUG_POSTFIX "d")

target_compile_features(${WEBPIER} PRIVATE cxx_std_17)
target_compile_features(${SLIPWAY} PRIVATE cxx_std_17)
target_compile_features(${CARRIER} PRIVATE cxx_std_17)

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
        set(WEBPIER_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
    else()
        if(CMAKE_INSTALL_PREFIX MATCHES "^(/usr)|(/usr/local)$")
            set(WEBPIER_INSTALL_BINDIR ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
            set(WEBPIER_INSTALL_DOCDIR ${CMAKE_INSTALL_DOCDIR})
        else()
            set(WEBPIER_INSTALL_BINDIR ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}/${CMAKE_INSTALL_BINDIR})
            set(WEBPIER_INSTALL_DOCDIR ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}/${CMAKE_INSTALL_DATADIR}/doc)
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
    target_link_libraries(${WEBPIER_TEST} PRIVATE Boost::unit_test_framework Boost::coroutine Boost::filesystem Boost::program_options "$<$<BOOL:${MSVC}>:Boost::property_tree>" "$<$<BOOL:${MSVC}>:Crypt32>" OpenSSL::SSL OpenSSL::Crypto)

    target_compile_features(${WEBPIER_TEST} PRIVATE cxx_std_17)
    target_compile_definitions(${WEBPIER_TEST} PRIVATE WEBPIER_MODULE="${WEBPIER_MODULE}" SLIPWAY_MODULE="${SLIPWAY_MODULE}" CARRIER_MODULE="${CARRIER_MODULE}")
    target_include_directories(${WEBPIER_TEST} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})
    set_target_properties(${WEBPIER_TEST} PROPERTIES DEBUG_POSTFIX "d")

    enable_testing()
    add_test(NAME ${WEBPIER_TEST} COMMAND ${WEBPIER_TEST} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

########## package ##########

if(NOT WEBPIER_SKIP_PACKAGE_RULES AND NOT WEBPIER_SKIP_INSTALL_RULES AND NOT CMAKE_SKIP_INSTALL_RULES)
    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
        cmake_host_system_information(RESULT DISTR QUERY DISTRIB_ID)
        if(DISTR MATCHES "(debian)|(ubuntu)|(kali)|(astra)")
            add_custom_command(DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog"
                COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")
            add_custom_target(changelog ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

            set(DOC_FILES
                ${CMAKE_CURRENT_BINARY_DIR}/changelog.gz
                ${CMAKE_CURRENT_SOURCE_DIR}/debian/copyright
                ${CMAKE_CURRENT_SOURCE_DIR}/debian/README.Debian
                )
            install(FILES ${DOC_FILES} DESTINATION ${WEBPIER_INSTALL_DOCDIR} COMPONENT debian_files)
            install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/webpier.png DESTINATION /usr/share/pixmaps COMPONENT debian_files)

            configure_and_generate(${CMAKE_CURRENT_SOURCE_DIR}/debian/webpier.conf.in ${CMAKE_CURRENT_BINARY_DIR}/webpier.conf)
            configure_and_generate(${CMAKE_CURRENT_SOURCE_DIR}/debian/webpier.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/webpier.desktop)

            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/webpier.conf DESTINATION /etc/webpier COMPONENT debian_files)
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/webpier.desktop DESTINATION /usr/share/applications COMPONENT debian_files)
        endif()
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")

        configure_and_generate(${CMAKE_CURRENT_SOURCE_DIR}/windows/modules.xml.in ${CMAKE_CURRENT_BINARY_DIR}/modules.xml)

        set(CPACK_PACKAGE_DESCRIPTION "The program for forwarding TCP services through NAT")
        set(CPACK_WIX_UPGRADE_GUID "FC2671A8-8282-4170-B58F-9FE385A18EAC")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
        set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/webpier.ico")
        set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/modules.xml")
        set(CPACK_COMPONENTS_ALL binary_files)
        set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
        set(CPACK_COMPONENT_BINARY_FILES_HIDDEN ON)

        set_property(INSTALL "${WEBPIER_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${WEBPIER}>" PROPERTY CPACK_START_MENU_SHORTCUTS "WebPier")
    endif()

    include(CPack)
endif()
