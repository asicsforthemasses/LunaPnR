
add_executable(lunapnr
    models/celllibmodel.cpp
    models/cellpinmodel.cpp
    common/guihelpers.cpp
    cellbrowser/cellbrowser.cpp
    cellbrowser/celllayoutview.cpp
    console/mmconsole.cpp
    src/mainwindow.cpp
    src/main.cpp)

find_package(Qt5 REQUIRED QUIET COMPONENTS Core Widgets)

target_link_libraries(lunapnr lunacore Qt5::Widgets)
target_include_directories(lunapnr PUBLIC ../core/include)
set_property(TARGET lunapnr PROPERTY AUTOMOC ON)
set_property(TARGET lunapnr PROPERTY AUTOUIC ON)
