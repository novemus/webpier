cmake_minimum_required(VERSION 3.5.0)
project(webpier VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    find_package(Boost REQUIRED COMPONENTS property_tree)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -fPIC -fvisibility=hidden")
    find_package(Boost)
endif()

set(wxWidgets_USE_STATIC ON)
find_package(wxWidgets REQUIRED core base propgrid)
if(wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif()

message("*** wxWidgets includes: ${wxWidgets_INCLUDE_DIRS}")
message("*** wxWidgets libraries: ${wxWidgets_LIBRARIES}")

add_executable(webpier WIN32 
    ui/main.cpp
    ui/mainframe.cpp
    ui/servicedialog.cpp
    ui/settingsdialog.cpp
    ui/importdialog.cpp
    ui/exportdialog.cpp
    ui/aboutdialog.cpp
    ui/messagedialog.cpp
    store/context.cpp
    )

target_include_directories(webpier PRIVATE ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(webpier PRIVATE ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})
