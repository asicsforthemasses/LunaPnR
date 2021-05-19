
add_executable(luna
    src/mainwindow.cpp
    src/main.cpp)

find_package(Qt5 REQUIRED QUIET COMPONENTS Core Widgets)

target_link_libraries(luna lunacore Qt5::Widgets)
set_property(TARGET luna PROPERTY AUTOMOC ON)
set_property(TARGET luna PROPERTY AUTOUIC ON)
