
find_package(Qt5 REQUIRED QUIET COMPONENTS Core Widgets LinguistTools)

# The qm files are generated in the build tree, but the qrc file is inside the
# source directory and the path to resources are relative to the location of
# the qrc file itself. We use configure_file() to copy the qrc file in the build
# directory such that it can find the qm translations files. The qrc file is
# copied if it doesn't exist in the destination or if it is modified.
#configure_file(translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)


set (LUNA_SRC_FILES
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
)

qt5_create_translation(QM_FILES ${LUNA_SRC_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/english.ts 
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/french.ts)

add_executable(lunapnr
    ${LUNA_SRC_FILES}
    resources/resources.qrc
    #${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
    ${QM_FILES}
    )

target_link_libraries(lunapnr lunacore Qt5::Widgets)
target_include_directories(lunapnr PUBLIC ../core/include .)

set_property(TARGET lunapnr PROPERTY AUTOMOC ON)
set_property(TARGET lunapnr PROPERTY AUTOUIC ON)
set_property(TARGET lunapnr PROPERTY AUTORCC ON)

