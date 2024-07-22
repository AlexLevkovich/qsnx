/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QSNXCLIENT_H
#define QSNXCLIENT_H

#include <QObject>
#include <QDBusPendingReply>

class ComAlexlQtQSNXInterface;

class QSNXClient : public QObject {
    Q_OBJECT
public:
    explicit QSNXClient(QObject *parent = nullptr);
    bool connect(const QString &url,const QString &certificate,bool backward,int port = 443);
    bool connect(const QString &url,const QString &username,const QString &password,bool backward,int port = 443);
    bool disconnect();
    bool isConnected() const;
    QString sessionInfo() const;
    void sendPassword(const QString & password);
    void terminate();

signals:
    void passwordRequested();
    void connecting();
    void connected();
    void disconnected();
    void error(const QString &str);

private:
    template<typename T> bool replyToValue(const QDBusPendingReply<T> & in_reply,T & ret) const {
        QSNXClient * p_this = (QSNXClient *)this;
        QDBusPendingReply<T> & reply = (QDBusPendingReply<T> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            emit p_this->error(reply.error().message());
            return false;
        }
        ret = reply.value();
        return true;
    }

    bool replyToVoid(const QDBusPendingReply<> & in_reply) const {
        QSNXClient * p_this = (QSNXClient *)this;
        QDBusPendingReply<> & reply = (QDBusPendingReply<> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            emit p_this->error(reply.error().message());
            return false;
        }
        return true;
    }

    ComAlexlQtQSNXInterface * m_interface;
};

#endif // QSNXCLIENT_H
