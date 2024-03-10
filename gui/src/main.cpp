// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "version.h"
#include <iostream>
#include <QApplication>
#include <QTranslator>
#include <QObject>
#include <QScreen>
#include <QDirIterator>
#include <QDebug>

#include <QCommandLineParser>
#include <clocale>

#include "mainwindow.h"
#include "common/logging.h"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("LunaPnR_Org");
    QCoreApplication::setApplicationName("LunaPnR");

    Logging::setLogLevel(Logging::LogType::INFO);

    QCommandLineParser parser;

    QCommandLineOption localeOption("locale", "Set locale name", "locale_value", "");
    parser.addOption(localeOption);

    QCommandLineOption showResourcesOption("show_resources", "Show qrc resources");
    parser.addOption(showResourcesOption);

    parser.process(app);

#if 0
    //if (parser.isSet(showResourcesOption))
    {
        std::cout << "Resources:\n";
        QDirIterator it(":", QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            auto s = it.next();
            std::cout << s.toStdString() << "\n";
        }
    }
#endif

    auto locale = QLocale();
    auto localeNameStr = locale.name().toStdString();

    QString localeOptionString = parser.value(localeOption);
    if (!localeOptionString.isEmpty())
    {
        localeNameStr = localeOptionString.toStdString();
        Logging::doLog(Logging::LogType::INFO,"Using locale %s\n", localeNameStr.c_str());
    }

    QTranslator lunapnrTranslator;
    auto langResourcePath = QString::asprintf(":/translations/lunapnr.%s.qm", localeNameStr.c_str());
    if (!lunapnrTranslator.load(langResourcePath))
    {
        Logging::doLog(Logging::LogType::WARNING,"Failed to load translator for locale: %s\n", localeNameStr.c_str());
    }

    if (!app.installTranslator(&lunapnrTranslator))
    {
        Logging::doLog(Logging::LogType::WARNING,"Failed to install translator for locale: %s\n", localeNameStr.c_str());
    }
    else
    {
        Logging::doLog(Logging::LogType::INFO,"Loaded translator for locale: %s\n", localeNameStr.c_str());
    }

    qApp->setStyle("fusion");

    // set the local to C again because Qt
    // so functions will use the period as the decimal indicator.
    // if we don't do this, the gates of hell will open.

    std::setlocale(LC_ALL, "C");            // for C and C++ where synced with stdio
    std::locale::global(std::locale("C"));  // for C++
    std::cout.imbue(std::locale());

    QLocale::setDefault(QLocale::C);

    QSettings settings;
    auto lastWindowSize = settings.value("application/size", QSize(0,0)).toSize();

    MainWindow window;
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();

    int height = screenGeometry.height();
    int width = screenGeometry.width();

    if (lastWindowSize.isNull())
    {
        // create main window and place it in the center
        // of the screen
        window.resize(width/2, height/2);
    }
    else
    {
        window.resize(lastWindowSize);
    }

    window.move((width - window.width()) / 2.0,
                    (height - window.height()) / 2.0);

    QString title = LUNAVERSIONSTRING;
    title.append(QObject::tr(" - a moonshot ASIC place&route tool"));
    window.setWindowTitle(title);
    window.show();

    // run application
    auto retval = app.exec();

    return retval;
}
