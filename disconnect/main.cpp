#include "../service/singleapplication.h"
#include "../client/qsnxclient.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("qsnx_disconnect");

    SingleApplication app(argc, argv,false,SingleApplication::System);
    QSNXClient m_client;
    qWarning() << "Connected. Disconnecting...";
    m_client.disconnect();
    return 0;
}
