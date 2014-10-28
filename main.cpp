/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */

#include <QtGui>

#include "window.h"
#include "generic.h"
#include "modem.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(datahub);

    QApplication app(argc, argv);

    QLocale::setDefault(QLocale::Greek);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1253"));
    QTextCodec::setCodecForTr( QTextCodec::codecForName("Windows-1253"));
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("Windows-1253") );

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, APP_NAME,
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    Window window;
    window.show();
    return app.exec();
}
