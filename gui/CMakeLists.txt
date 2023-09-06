## SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
##
## SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.20)

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

find_package(Qt6 REQUIRED QUIET COMPONENTS Core Widgets Gui Network)
find_package(PythonLibs 3.8 REQUIRED COMPONENTS Interpreter Development)

########################################################################
# Add TomlPlusPlus
########################################################################

CPMAddPackage("gh:marzer/tomlplusplus#v3.3.0")

# The qm files are generated in the build tree, but the qrc file is inside the
# source directory and the path to resources are relative to the location of
# the qrc file itself. We use configure_file() to copy the qrc file in the build
# directory such that it can find the qm translations files. The qrc file is
# copied if it doesn't exist in the destination or if it is modified.
#configure_file(translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

# add a dependency to make console/commandcompletion.cpp using the
# doctool from the tools directory

add_custom_target(commandcompletion
    $<TARGET_FILE:doctool> ${CMAKE_CURRENT_SOURCE_DIR}/python/pyluna_extra.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../core/python/pylunapnr.cpp ${CMAKE_CURRENT_SOURCE_DIR}/console/commandcompletion.inc
    BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/console/commandcompletion.inc
    VERBATIM
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/python/pyluna_extra.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../core/python/pylunapnr.cpp doctool
    USES_TERMINAL
    )

set (LUNA_SRC_FILES
    models/modelnodebase.cpp
    models/modulemodel.cpp
    models/moduleinfomodel.cpp
    models/celllibmodel.cpp
    models/cellinfomodel.cpp
    models/layerappearancetablemodel.cpp
    models/techlibinfomodel.cpp
    models/filesetupmodel.cpp
    models/techlibsitemodel.cpp
    common/guihelpers.cpp
    common/hatchlibrary.cpp
    common/layerrenderinfo.cpp
    common/database.cpp
    common/projectsetup.cpp
    common/tasklist.cpp
    common/pdkinfo.cpp
    widgets/colorbutton.cpp
    widgets/hatchbutton.cpp
    widgets/hatchdialog.cpp
    widgets/hatchdisplay.cpp
    widgets/txtoverlay.cpp
    widgets/clickablelabel.cpp
    widgets/blockframe.cpp
    widgets/flattogglebutton.cpp
    widgets/flatimage.cpp
    widgets/flatimagebutton.cpp
    widgets/flattilebase.cpp
    widgets/flatactiontile.cpp
    widgets/blockcontainer.cpp
    widgets/rectdelegate.cpp
    widgets/layerwidget.cpp
    floorplanview/floorplanview.cpp
    techbrowser/techbrowser.cpp
    designbrowser/designbrowser.cpp
    cellbrowser/cellbrowser.cpp
    cellbrowser/celllayoutview.cpp
    propertyview/propertyview.cpp
    aboutdialog/aboutdialog.cpp
    projectmanager/filesetupmanager.cpp
    projectmanager/projectmanager.cpp
    floorplandialog/floorplandialog.cpp
    ctsdialog/ctsdialog.cpp
    pdkdialog/pdkdialog.cpp
    pdkdialog/pdktile.cpp
    console/mmconsole.cpp
    console/mtstringbuffer.cpp
    console/singlelineedit.cpp
    opensta/openstaparser.cpp
    tasks/tasks.cpp
    tasks/readallfiles.cpp
    tasks/checktiming.cpp
    tasks/preflightchecks.cpp
    tasks/createfloorplan.cpp
    tasks/place.cpp
    tasks/ctstask.cpp
    tasks/globalroute.cpp
    python/types/pyprojectlef.cpp
    python/types/pyprojectlib.cpp
    python/types/pyprojectverilog.cpp
    python/pyluna_extra.cpp
    pdkinstaller/pdkinstaller.cpp
    pdkinstaller/downloader.cpp
    src/configurationdialog.cpp
    src/mainwindow.cpp
    src/main.cpp
)

#set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/translations/lunapnr.en_US.ts
#    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations")

#qt5_create_translation(QM_FILES ${LUNA_SRC_FILES}
#    ${CMAKE_CURRENT_SOURCE_DIR}/translations/lunapnr.en_US.ts
#    ${CMAKE_CURRENT_SOURCE_DIR}/translations/lunapnr.nl_NL.ts
#)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

#message(${QM_FILES})

add_executable(lunapnr
    ${LUNA_SRC_FILES}
    resources/resources.qrc
    #${CMAKE_CURRENT_BINARY_DIR}/english.qm
    #${QM_FILES}
    )

add_dependencies(lunapnr commandcompletion)

target_link_libraries(lunapnr lunacore Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Network tomlplusplus::tomlplusplus)
target_include_directories(lunapnr PUBLIC ../core/include ${PYTHON_INCLUDE_DIRS} .)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
#target_compile_options(lunapnr PUBLIC -fno-pie -no-pie)
#target_link_options(lunapnr PUBLIC -fno-pie -no-pie)

set_property(TARGET lunapnr PROPERTY AUTOMOC ON)
set_property(TARGET lunapnr PROPERTY AUTOUIC ON)
set_property(TARGET lunapnr PROPERTY AUTORCC ON)
