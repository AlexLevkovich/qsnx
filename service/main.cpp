/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "qsnxservice.h"
#include <QDBusConnection>
#include "singleapplication.h"
#include "sigwatch.h"
#include <unistd.h>
#include <QTranslator>
#include <QLibraryInfo>


int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("qsnx_service");

    SingleApplication app(argc, argv,false,SingleApplication::System);

    if (getuid() != 0) {
        qCritical() << "You must be root!!!";
        return 1;
    }

    if (!QDBusConnection::systemBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus.\n"
                       "Please check your system settings and try again";
        return 1;
    }

    QTranslator * m_translator = new QTranslator(QCoreApplication::instance());
    if(!m_translator->load(QLocale::system(),"qsnx_service","_",TRANS_DIR1)) m_translator->load(QLocale::system(),"qsnx_service","_",TRANS_DIR2);
    QCoreApplication::installTranslator(m_translator);
    QTranslator * m_translator2 = new QTranslator(QCoreApplication::instance());
    if (m_translator2->load(QLocale::system(),"qt","_",QLibraryInfo::location(QLibraryInfo::TranslationsPath))) QCoreApplication::installTranslator(m_translator2);

    QSNXService service;

    UnixSignalWatcher sigwatch;
    sigwatch.watchForSignal(SIGINT);
    sigwatch.watchForSignal(SIGTERM);
    sigwatch.watchForSignal(SIGQUIT);
    sigwatch.watchForSignal(SIGHUP);
    QObject::connect(&sigwatch,&UnixSignalWatcher::unixSignal,&app,&SingleApplication::quit);

    return app.exec();
}
