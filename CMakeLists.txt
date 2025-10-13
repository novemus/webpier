message("***** webpier ***** ")

########## setup ##########

cmake_minimum_required(VERSION 3.21)

project(webpier VERSION 1.0.1 DESCRIPTION "The program for forwarding TCP applications via NAT" LANGUAGES CXX)

option(WEBPIER_SKIP_TEST_RULES "Don't generate test rules" ON)
option(WEBPIER_SKIP_INSTALL_RULES "Don't generate install rules" OFF)
option(WEBPIER_SKIP_PACKAGE_RULES "Don't generate package rules" ON)

########## dependencies ##########

if(MSVC)
    if(NOT WEBPIER_SKIP_TEST_RULES)
        find_package(Boost 1.80 CONFIG REQUIRED COMPONENTS filesystem process property_tree coroutine program_options unit_test_framework)
    else()
        find_package(Boost 1.80 CONFIG REQUIRED COMPONENTS filesystem process property_tree coroutine program_options)
    endif()
else()
    if (NOT BUILD_SHARED_LIBS)
        set(Boost_USE_STATIC_LIBS ON)
    endif()
    if(NOT WEBPIER_SKIP_TEST_RULES)
        find_package(Boost 1.80 CONFIG REQUIRED COMPONENTS filesystem coroutine program_options unit_test_framework)
    else()
        find_package(Boost 1.80 CONFIG REQUIRED COMPONENTS filesystem coroutine program_options)
    endif()
    if (Boost_VERSION VERSION_GREATER_EQUAL 1.88)
        find_package(Boost CONFIG REQUIRED COMPONENTS process)
    endif()
endif()

message("*** Boost Version: ${Boost_VERSION}")
message("*** Boost Include Dirs: ${Boost_INCLUDE_DIRS}")
message("*** Boost Librariy Dirs: ${Boost_LIBRARY_DIRS}")
message("*** Boost Libraries: ${Boost_LIBRARIES}")

find_package(wxWidgets COMPONENTS core base propgrid net QUIET)

if (NOT wxWidgets_FOUND)
    find_package(wxWidgets CONFIG REQUIRED core base propgrid net)
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

find_package(plexus 3.2 REQUIRED)

message("*** plexus Version: ${plexus_VERSION}")
message("*** plexus Include: ${plexus_INCLUDE_DIRS}")
message("*** plexus Library: ${plexus_LIBRARY}")

find_package(wormhole 1.2.4 REQUIRED)

message("*** wormhole Version: ${wormhole_VERSION}")
message("*** wormhole Include: ${wormhole_INCLUDE_DIRS}")
message("*** wormhole Library: ${wormhole_LIBRARY}")

find_package(tubus 1.2.3 REQUIRED DEBUG)

message("*** tubus Version: ${tubus_VERSION}")
message("*** tubus Include: ${tubus_INCLUDE_DIRS}")
message("*** tubus Library: ${tubus_LIBRARY}")

find_package(opendht 3.2 REQUIRED)
find_package(fmt REQUIRED)

find_package(msgpackc-cxx QUIET CONFIG NAMES msgpackc-cxx msgpack)
if(msgpackc-cxx_FOUND)
  add_library(msgpack-cxx ALIAS msgpackc-cxx)
else()
  find_package(msgpack-cxx CONFIG REQUIRED)
endif()

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
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/add.png ${CMAKE_CURRENT_BINARY_DIR}/assets/add.h add_btn_png)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/edit.png ${CMAKE_CURRENT_BINARY_DIR}/assets/edit.h edit_btn_png)
    convert_to_cpp_array(${CMAKE_CURRENT_SOURCE_DIR}/assets/delete.png ${CMAKE_CURRENT_BINARY_DIR}/assets/delete.h delete_btn_png)
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
    src/backend/server.h
    src/backend/server.cpp
    src/backend/main.cpp
    src/store/utils.h
    src/store/utils.cpp
    )

target_sources(${CARRIER} PRIVATE
    src/store/utils.h
    src/store/utils.cpp
    src/backend/tunnel.cpp
    )

target_link_libraries(${WEBPIER} PRIVATE ${wxWidgets_LIBRARIES} Boost::coroutine Boost::filesystem Boost::program_options "$<$<BOOL:${MSVC}>:Boost::property_tree>" "$<$<BOOL:${MSVC}>:Crypt32>" plexus::libplexus OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${CARRIER} PRIVATE Boost::filesystem Boost::program_options "$<$<BOOL:${MSVC}>:Crypt32>" wormhole::libwormhole tubus::libtubus OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${SLIPWAY} PRIVATE Boost::coroutine Boost::filesystem Boost::program_options $<$<VERSION_GREATER_EQUAL:${Boost_VERSION},1.88>:Boost::process> "$<$<BOOL:${MSVC}>:Boost::property_tree>" "$<$<BOOL:${MSVC}>:Crypt32>" plexus::libplexus wormhole::libwormhole tubus::libtubus opendht fmt::fmt msgpack-cxx PkgConfig::GnuTLS PkgConfig::argon2 PkgConfig::Nettle PkgConfig::Jsoncpp OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${WEBPIER} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS})
target_include_directories(${SLIPWAY} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})
target_include_directories(${CARRIER} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})

set_target_properties(${WEBPIER} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${SLIPWAY} PROPERTIES DEBUG_POSTFIX "d" CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
set_target_properties(${CARRIER} PROPERTIES DEBUG_POSTFIX "d" CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)

target_compile_features(${WEBPIER} PRIVATE cxx_std_17)
target_compile_features(${SLIPWAY} PRIVATE cxx_std_17)
target_compile_features(${CARRIER} PRIVATE cxx_std_17)

if(NOT WEBPIER_SKIP_PACKAGE_RULES)
    if(WIN32)
        set(WEBPIER_CONFIG "Software\\\\WebPier")
    elseif(APPLE)
        set(WEBPIER_CONFIG "$<IF:$<BOOL:${CPACK_PACKAGING_INSTALL_PREFIX}>,${CPACK_PACKAGING_INSTALL_PREFIX},${CMAKE_INSTALL_PREFIX}>/WebPier.app/Contents/Resources/webpier.conf")
    else()
        set(WEBPIER_CONFIG "/etc/webpier/webpier.conf")
    endif()
    target_compile_definitions(${WEBPIER} PRIVATE WEBPIER_CONFIG="${WEBPIER_CONFIG}")
    target_compile_definitions(${SLIPWAY} PRIVATE WEBPIER_CONFIG="${WEBPIER_CONFIG}")
    target_compile_definitions(${CARRIER} PRIVATE WEBPIER_CONFIG="${WEBPIER_CONFIG}")
endif()

target_compile_definitions(${WEBPIER} PRIVATE WEBPIER_VERSION="${PROJECT_VERSION}")

add_definitions(-DWEBPIER_FILE_NAME="$<TARGET_FILE_NAME:${WEBPIER}>" -DSLIPWAY_FILE_NAME="$<TARGET_FILE_NAME:${SLIPWAY}>" -DCARRIER_FILE_NAME="$<TARGET_FILE_NAME:${CARRIER}>")

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601 -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -DTASKXML_FILE_NAME="task.xml")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windows/task.xml" "${CMAKE_CURRENT_BINARY_DIR}/task.xml" COPYONLY)
endif()

########## install ##########

if(NOT WEBPIER_SKIP_INSTALL_RULES AND NOT CMAKE_SKIP_INSTALL_RULES)

    include(GNUInstallDirs)

    install(TARGETS ${WEBPIER} RUNTIME DESTINATION "$<IF:$<BOOL:${APPLE}>,WebPier.app/Contents/Resources/Modules,${CMAKE_INSTALL_BINDIR}>" COMPONENT binary_files)
    install(TARGETS ${SLIPWAY} RUNTIME DESTINATION "$<IF:$<BOOL:${APPLE}>,WebPier.app/Contents/Resources/Modules,${CMAKE_INSTALL_BINDIR}>" COMPONENT binary_files)
    install(TARGETS ${CARRIER} RUNTIME DESTINATION "$<IF:$<BOOL:${APPLE}>,WebPier.app/Contents/Resources/Modules,${CMAKE_INSTALL_BINDIR}>" COMPONENT binary_files)

    if(WIN32)
        install(FILES "$<TARGET_PDB_FILE:${WEBPIER}>" COMPONENT pdb_files
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL)
        install(FILES "$<TARGET_PDB_FILE:${SLIPWAY}>" COMPONENT pdb_files
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL)
        install(FILES "$<TARGET_PDB_FILE:${CARRIER}>" COMPONENT pdb_files
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/windows/task.xml DESTINATION ${CMAKE_INSTALL_DATADIR} COMPONENT windows_files)
    endif()
endif()

########## tests ##########

if(NOT WEBPIER_SKIP_TEST_RULES)
    set(WEBPIER_TEST webpier_ut)
    add_executable(${WEBPIER_TEST} tests/utils.cpp tests/context.cpp tests/message.cpp tests/slipway.cpp src/store/context.cpp src/backend/message.cpp src/backend/client.cpp src/backend/server.cpp src/store/utils.cpp)
    target_link_libraries(${WEBPIER_TEST} PRIVATE Boost::unit_test_framework Boost::coroutine Boost::filesystem Boost::program_options $<$<VERSION_GREATER_EQUAL:${Boost_VERSION},1.88>:Boost::process> "$<$<BOOL:${MSVC}>:Boost::property_tree>" "$<$<BOOL:${MSVC}>:Crypt32>" plexus::libplexus wormhole::libwormhole opendht fmt::fmt msgpack-cxx PkgConfig::GnuTLS PkgConfig::argon2 PkgConfig::Nettle PkgConfig::Jsoncpp OpenSSL::SSL OpenSSL::Crypto)

    target_compile_features(${WEBPIER_TEST} PRIVATE cxx_std_17)
    set_target_properties(${WEBPIER_TEST} PROPERTIES DEBUG_POSTFIX "d" CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
    target_include_directories(${WEBPIER_TEST} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" ${Boost_INCLUDE_DIRS})

    enable_testing()
    add_test(NAME ${WEBPIER_TEST} COMMAND ${WEBPIER_TEST} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

########## package ##########

if(NOT WEBPIER_SKIP_PACKAGE_RULES AND NOT WEBPIER_SKIP_INSTALL_RULES AND NOT CMAKE_SKIP_INSTALL_RULES)

    set(CPACK_PACKAGE_NAME "webpier")
    set(CPACK_PACKAGE_VENDOR "Community")
    set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An application for forwarding TCP services through NAT")
    set(CPACK_PACKAGE_DESCRIPTION "The 'webpier' app allows to forward non-public TCP services\nvia UDP tunnels without using third-party relay servers.")
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)

    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/webpier.png DESTINATION /usr/share/pixmaps COMPONENT linux_files)

        configure_and_generate("${CMAKE_CURRENT_SOURCE_DIR}/linux/webpier.conf.in" "${CMAKE_CURRENT_BINARY_DIR}/webpier.conf")
        configure_and_generate("${CMAKE_CURRENT_SOURCE_DIR}/linux/webpier.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/webpier.desktop")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/webpier.conf" DESTINATION /etc/webpier COMPONENT linux_files)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/webpier.desktop" DESTINATION /usr/share/applications COMPONENT linux_files)

        set(CPACK_COMPONENTS_ALL "binary_files;linux_files")
        set(CPACK_COMPONENT_BINARY_FILES_HIDDEN ON)
        set(CPACK_COMPONENT_LINUX_FILES_HIDDEN ON)

        set(CPACK_RPM_PACKAGE_VENDOR ${CPACK_PACKAGE_VENDOR})
        set(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
        set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
        set(CPACK_RPM_PACKAGE_RECOMMENDS "cronie")
        set(CPACK_RPM_PACKAGE_AUTOREQ 1)
        set(CPACK_RPM_CHANGELOG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/linux/changelog.rpm)
        set(CPACK_RPM_PACKAGE_LICENSE "Apache 2.0")
        set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
        set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
        set(CPACK_RPM_DEBUGINFO_SINGLE_PACKAGE ON)
        set(CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/novemus/webpier")

        set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
        set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        set(CPACK_DEBIAN_PACKAGE_SECTION "admin")
        set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
        set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "cron")
        set(CPACK_DEB_COMPONENT_INSTALL ON)

        cmake_host_system_information(RESULT DISTR QUERY DISTRIB_ID)
        if(DISTR MATCHES "(debian)|(ubuntu)|(kali)|(astra)")
            add_custom_command(DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linux/changelog"
                COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/linux/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")
            add_custom_target(changelog ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

            set(DOC_FILES
                ${CMAKE_CURRENT_BINARY_DIR}/changelog.gz
                ${CMAKE_CURRENT_SOURCE_DIR}/linux/copyright
                ${CMAKE_CURRENT_SOURCE_DIR}/linux/README.Debian
                )
            install(FILES ${DOC_FILES} DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT linux_files)
        endif()
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        configure_and_generate("${CMAKE_CURRENT_SOURCE_DIR}/windows/modules.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/modules.xml")

        set(CPACK_ARCHIVE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CMAKE_PROJECT_VERSION}-pdb")
        set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

        set(CPACK_WIX_UPGRADE_GUID "FC2671A8-8282-4170-B58F-9FE385A18EAC")
        set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/webpier.ico")
        set(CPACK_WIX_EXTENSIONS "WixFirewallExtension")
        set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/modules.xml")
        set(CPACK_WIX_UI_REF "WixUI_InstallDir")
        set(CPACK_COMPONENTS_ALL "binary_files;windows_files")
        set(CPACK_COMPONENT_BINARY_FILES_HIDDEN ON)
        set(CPACK_COMPONENT_WINDOWS_FILES_HIDDEN ON)

        set_property(INSTALL "${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${WEBPIER}>" PROPERTY CPACK_START_MENU_SHORTCUTS "WebPier")
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
        
        if (NOT CPACK_PACKAGE_RELEASE)
            set(CPACK_PACKAGE_RELEASE 1)
        endif()

        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}-${CPACK_PACKAGE_RELEASE}_${CMAKE_HOST_SYSTEM_NAME}")

        if(NOT CPACK_PRODUCTBUILD_IDENTIFIER)
            set(CPACK_PRODUCTBUILD_IDENTIFIER "dev.community.webpier")
        endif()

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/darwin/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
        configure_and_generate("${CMAKE_CURRENT_SOURCE_DIR}/darwin/webpier.conf.in" "${CMAKE_CURRENT_BINARY_DIR}/webpier.conf")
        configure_and_generate("${CMAKE_CURRENT_SOURCE_DIR}/darwin/postinst.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/postinst.sh")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" DESTINATION WebPier.app/Contents COMPONENT darwin_files)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/webpier.conf" DESTINATION WebPier.app/Contents/Resources COMPONENT darwin_files)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/webpier.icns" DESTINATION WebPier.app/Contents/Resources COMPONENT darwin_files)

        set(CPACK_COMPONENTS_ALL "binary_files;darwin_files")
        set(CPACK_COMPONENT_BINARY_FILES_HIDDEN ON)
        set(CPACK_COMPONENT_DARWIN_FILES_HIDDEN ON)
        set(CPACK_POSTFLIGHT_BINARY_FILES_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/postinst.sh")

        set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/darwin/welcome.txt")
        set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/darwin/readme.txt")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
    endif()

    include(CPack)
endif()
