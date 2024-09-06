/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "qsnxclient.h"
#include "qsnx_interface.h"
#include <QFileInfo>

QSNXClient::QSNXClient(QObject *parent) : QObject(parent) {
    m_interface = new ComAlexlQtQSNXInterface(ComAlexlQtQSNXInterface::staticInterfaceName(),"/",QDBusConnection::systemBus(),this);
    if (QFileInfo((INSTALL_PREFIX"/bin/"+QFileInfo(QCoreApplication::applicationFilePath()).fileName())).canonicalFilePath() == QFileInfo(QCoreApplication::applicationFilePath()).canonicalFilePath()) m_interface->connection().interface()->startService(ComAlexlQtQSNXInterface::staticInterfaceName());

    QObject::connect(m_interface,&ComAlexlQtQSNXInterface::passwordRequested,this,&QSNXClient::passwordRequested);
    QObject::connect(m_interface,&ComAlexlQtQSNXInterface::connecting,this,&QSNXClient::connecting);
    QObject::connect(m_interface,&ComAlexlQtQSNXInterface::connected,this,&QSNXClient::connected);
    QObject::connect(m_interface,&ComAlexlQtQSNXInterface::disconnected,this,&QSNXClient::disconnected);
    QObject::connect(m_interface,&ComAlexlQtQSNXInterface::error,this,&QSNXClient::error);
}

bool QSNXClient::connect(const QString &url,const QString &certificate,bool backward,int port) {
    if (!m_interface->isValid()) return false;

    replyToVoid(m_interface->connect(url,certificate,port,backward));
    return true;
}

bool QSNXClient::connect(const QString &url,const QString &username,const QString &password,bool backward,int port) {
    if (!m_interface->isValid()) return false;

    replyToVoid(m_interface->connect(url,username,password,port,backward));
    return true;
}

bool QSNXClient::disconnect() {
    if (!m_interface->isValid()) return false;

    replyToVoid(m_interface->disconnect());
    return true;
}

bool QSNXClient::isConnected() const {
    if (!m_interface->isValid()) return false;

    bool ret = false;
    if (replyToValue<bool>(m_interface->isConnected(),ret)) return ret;
    return ret;
}

bool QSNXClient::hasBackwardCompabilityOption() const {
    if (!m_interface->isValid()) return false;

    bool ret = false;
    if (replyToValue<bool>(m_interface->hasBackwardCompabilityOption(),ret)) return ret;
    return ret;
}

QString QSNXClient::sessionInfo() const {
    if (!m_interface->isValid()) return QString();

    QString ret;
    if (replyToValue<QString>(m_interface->isConnected(),ret)) return ret;
    return ret;
}

void QSNXClient::sendPassword(const QString & password) {
    if (!m_interface->isValid()) return;

    replyToVoid(m_interface->sendPassword(password));
}

void QSNXClient::terminate() {
    if (!m_interface->isValid()) return;

    replyToVoid(m_interface->terminate());
}
