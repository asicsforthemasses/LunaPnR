
add_executable(lunapnr
    models/modelnodebase.cpp
    models/celllibmodel.cpp
    models/cellinfomodel.cpp
    models/techlibinfomodel.cpp
    common/guihelpers.cpp
    common/layerrenderinfo.cpp
    widgets/colorbutton.cpp
    floorplanview/floorplanview.cpp
    techbrowser/techbrowser.cpp
    cellbrowser/cellbrowser.cpp
    cellbrowser/celllayoutview.cpp
    propertyview/propertyview.cpp
    console/mmconsole.cpp
    src/mainwindow.cpp
    src/main.cpp
    resources/resources.qrc)

find_package(Qt5 REQUIRED QUIET COMPONENTS Core Widgets)

target_link_libraries(lunapnr lunacore Qt5::Widgets)
target_include_directories(lunapnr PUBLIC ../core/include .)

set_property(TARGET lunapnr PROPERTY AUTOMOC ON)
set_property(TARGET lunapnr PROPERTY AUTOUIC ON)
set_property(TARGET lunapnr PROPERTY AUTORCC ON)

