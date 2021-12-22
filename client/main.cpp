/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "../service/singleapplication.h"
#include "qsnxwindow.h"
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("qsnx_client");

    QIcon::setFallbackThemeName("qsnx");
    SingleApplication app(argc, argv,false,SingleApplication::System);

    QTranslator * m_translator = new QTranslator(QCoreApplication::instance());
    if(!m_translator->load(QLocale::system(),"qsnx_client","_",TRANS_DIR1)) m_translator->load(QLocale::system(),"qsnx_client","_",TRANS_DIR2);
    QCoreApplication::installTranslator(m_translator);
    QTranslator * m_translator2 = new QTranslator(QCoreApplication::instance());
    if (m_translator2->load(QLocale::system(),"qt","_",QLibraryInfo::location(QLibraryInfo::TranslationsPath))) QCoreApplication::installTranslator(m_translator2);

    app.setQuitOnLastWindowClosed(false);
    QSNXWindow::showTray();
    return app.exec();
}
