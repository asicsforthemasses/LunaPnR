#include <iostream>
#include <QApplication>
#include <QTranslator>
#include <QObject>
#include <QDesktopWidget>
#include <QDirIterator>
#include <QDebug>

#include <QCommandLineParser>
#include <clocale>

#include "mainwindow.h"
#include "common/logging.h"

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);

    setLogLevel(LOG_INFO);
    
    QCommandLineParser parser;
        
    QCommandLineOption localeOption("locale", "Set locale name", "locale_value", "");
    parser.addOption(localeOption);

    QCommandLineOption showResourcesOption("show_resources", "Show qrc resources");
    parser.addOption(showResourcesOption);

    parser.process(app);

    if (parser.isSet(showResourcesOption))
    {
        std::cout << "Resources:\n";
        QDirIterator it(":", QDirIterator::Subdirectories);
        while (it.hasNext()) 
        {
            auto s = it.next();
            std::cout << s.toStdString() << "\n";
        }    
    }

    auto locale = QLocale();
    auto localeNameStr = locale.name().toStdString();
    
    QString localeOptionString = parser.value(localeOption);
    if (!localeOptionString.isEmpty())
    {
        localeNameStr = localeOptionString.toStdString();
        doLog(LOG_INFO,"Using locale %s\n", localeNameStr.c_str());
    }
    
    QTranslator lunapnrTranslator;
    auto langResourcePath = QString::asprintf(":/translations/lunapnr.%s.qm", localeNameStr.c_str());
    if (!lunapnrTranslator.load(langResourcePath))
    {
        doLog(LOG_WARN,"Failed to load translator for locale: %s\n", localeNameStr.c_str());
    }

    if (!app.installTranslator(&lunapnrTranslator))
    {        
        doLog(LOG_WARN,"Failed to install translator for locale: %s\n", localeNameStr.c_str());
    }
    else
    {
        doLog(LOG_INFO,"Loaded translator for locale: %s\n", localeNameStr.c_str());
    }

    qApp->setStyle("fusion");

    // set the local to C again because Qt 
    // so functions will use the period as the decimal indicator.
    // if we don't do this, the gates of hell will open.

    std::setlocale(LC_ALL, "C");            // for C and C++ where synced with stdio
    std::locale::global(std::locale("C"));  // for C++
    std::cout.imbue(std::locale());

    QLocale::setDefault(QLocale::C);

    // create main window and place it in the center
    // of the screen
    MainWindow window;
    QDesktopWidget widget;
    QRect screenGeometry = widget.screenGeometry();

    int height = screenGeometry.height();
    int width = screenGeometry.width();

    window.resize(width/2, height/2);

    window.move((width - window.width()) / 2.0,
                    (height - window.height()) / 2.0);

    window.setWindowTitle(QObject::tr("LunaPnR - a moonshot ASIC place&route tool"));
    window.show();

    // run application
    auto retval = app.exec();

    return retval;
}
