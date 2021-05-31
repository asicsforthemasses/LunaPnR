#include <iostream>
#include <QApplication>
#include <QDesktopWidget>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);

    qApp->setStyle("fusion");

    // set the local to C again because Qt 
    // changes this..
    std::setlocale(LC_ALL, "C");            // for C and C++ where synced with stdio
    std::locale::global(std::locale("C"));  // for C++
    std::cout.imbue(std::locale());

    QLocale::setDefault(QLocale::C);

    MainWindow window;

    QDesktopWidget widget;
    QRect screenGeometry = widget.screenGeometry();

    int height = screenGeometry.height();
    int width = screenGeometry.width();

    window.move((width - window.width()) / 2.0,
                    (height - window.height()) / 2.0);

    window.resize(640, 480);
    window.setWindowTitle("LUNA - a moonshot ASIC place&route tool" );
    window.show();

    // run application
    auto retval = app.exec();

    return retval;
}
