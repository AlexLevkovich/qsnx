/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QSNXSERVICE_H
#define QSNXSERVICE_H

#include <QObject>
#include <QDBusContext>
#include "qptyprocess.h"

class SNXProcess: public QObject {
    Q_OBJECT
public:
    SNXProcess(const QString &url,const QString &certificate,int port,QObject *parent = nullptr);
    SNXProcess(const QString &url,const QString &username,const QString &password,int port = 443,QObject *parent = nullptr);
    SNXProcess(QObject *parent = nullptr);
    QString errorString() const;
    void start();
    void sendPassword(const QString & password);
    bool startDetached();
    void terminate();
    bool isRunning() const;
    bool isConnected() const;
    QStringList dnsSuffixes() const;
    QStringList dnsIPs() const;
    qint64 processId() const;
    QString connnectedInfo() const;
    static qint64 processId(const QString & path);
    static qint64 startDetached(const QString & pgm,const QStringList & args);

signals:
    void passwordRequested();
    void connected();
    void forked();
    void disconnected();
    void errorOccurred(const QString & error);

private slots:
    void snx_forked();

private:
    ~SNXProcess();
    void init();
    bool check_parameters();
    void analyze_line(const QByteArray & line);
    bool write_data(const char * data);
    static const QString user_dir();

    QPtyProcess m_process;
    QString m_password;
    QString m_username;
    QString m_certificate;
    QString m_connected_info;
    QStringList dns_ips;
    QStringList dns_suffixes;
    QString m_error;
    bool m_first_time;
    bool m_is_connected;
};

class QSNXService : public QObject, QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.alexl.qt.QSNX")
public:
    explicit QSNXService(QObject *parent = nullptr);
    ~QSNXService();

public slots:
    void connect(const QString &url,const QString &certificate,int port);
    void connect(const QString &url,const QString &username,const QString &password,int port);
    void disconnect();
    bool isConnected();
    QString sessionInfo();
    void sendPassword(const QString & password);
    void terminate();
signals:
    void passwordRequested();
    void connected();
    void disconnected();
    void error(const QString &str);

private:
    void start_process(SNXProcess * process);

    SNXProcess * m_process;
};

#endif // QSNXSERVICE_H
