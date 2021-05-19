
add_executable(lunapnr
    src/mainwindow.cpp
    src/main.cpp)

find_package(Qt5 REQUIRED QUIET COMPONENTS Core Widgets)

target_link_libraries(lunapnr lunacore Qt5::Widgets)
set_property(TARGET lunapnr PROPERTY AUTOMOC ON)
set_property(TARGET lunapnr PROPERTY AUTOUIC ON)
