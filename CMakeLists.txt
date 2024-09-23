cmake_minimum_required(VERSION 3.5.0)
project(webpier VERSION 0.1.0 LANGUAGES C CXX)

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

target_include_directories(webpier PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(webpier PRIVATE ${wxWidgets_LIBRARIES})
