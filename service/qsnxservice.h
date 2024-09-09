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
    SNXProcess(const QString &url,const QString &certificate,int port,bool backward,QObject *parent = nullptr);
    SNXProcess(const QString &url,const QString &username,const QString &password,int port,bool backward,QObject *parent = nullptr);
    SNXProcess(QObject *parent = nullptr);
    QString errorString() const;
    void sendPassword(const QString & password);
    void start();
    bool startDetached();
    void terminate();
    bool isRunning() const;
    bool isConnected() const;
    QString url() const;
    QString ip() const;
    QStringList dnsSuffixes() const;
    QStringList dnsIPs() const;
    qint64 processId() const;
    QString connnectedInfo() const;
    int maxConnectCount() const;
    void setMaxConnectCount(int count);
    static qint64 processId(const QString & path);
    static qint64 startDetached(const QString & pgm,const QStringList & args);

signals:
    void passwordRequested();
    void connecting();
    void connected(const QString & ip,const QStringList & dns_ips,const QStringList & dns_suffixes);
    void disconnected();
    void errorOccurred(const QString & error);

private:
    enum State {
        Starting,
        SortOfStarted,
        Started,
        Stopped
    };

    ~SNXProcess();
    void snx_forked();
    void init();
    bool check_parameters();
    void analyze_line(const QByteArray & line);
    bool write_data(const char * data);
    static const QString user_dir();

    QPtyProcess m_process;
    QString m_url;
    QString m_password;
    QString m_username;
    QString m_certificate;
    QString m_connected_info;
    QString m_ip;
    QStringList m_dns_ips;
    QStringList m_dns_suffixes;
    QString m_error;
    bool m_first_time;
    State m_state;
    int m_connect_counter;
    int m_max_connect_count;
    static const int MAX_CONNECT_COUNT;
};

class QSNXService : public QObject, QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.alexl.qt.QSNX")
public:
    explicit QSNXService(QObject *parent = nullptr);
    ~QSNXService();

public slots:
    void connect(const QString &url,const QString &certificate,int port,bool backward);
    void connect(const QString &url,const QString &username,const QString &password,int port,bool backward);
    void disconnect();
    bool isConnected();
    void sendPassword(const QString & password);
    void terminate();
    bool hasBackwardCompabilityOption();
signals:
    void passwordRequested();
    void connecting();
    void connected(const QString & ip,const QStringList & dns_ips,const QStringList & dns_suffixes);
    void disconnected();
    void error(const QString &str);

private:
    void start_process(SNXProcess * process);
    bool check_option(const QString & option);

    SNXProcess * m_process;
};

#endif // QSNXSERVICE_H
