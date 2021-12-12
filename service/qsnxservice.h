#ifndef QSNXSERVICE_H
#define QSNXSERVICE_H

#include <QObject>
#include <QDBusContext>
#include <QProcess>
#include <QTemporaryFile>

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
    qint64 processId() const;
    QString connnectedInfo() const;
    static qint64 processId(const QString & path);

signals:
    void passwordRequested();
    void connected();
    void disconnected();
    void errorOccurred(const QString & error);

private slots:
    void forked();

private:
    void init();
    bool check_parameters();
    void analyze_line(const QByteArray & line);
    bool write_data(const char * data);

    QProcess m_process;
    QString m_password;
    QString m_username;
    QString m_certificate;
    QString m_connected_info;
    QString m_error;
    bool m_first_time;
    bool m_is_connected;
};

class QSNXService : public QObject, QDBusContext {
    Q_OBJECT
public:
    explicit QSNXService(QObject *parent = nullptr);
    ~QSNXService();
    Q_INVOKABLE void connect(const QString &url,const QString &certificate,int port);
    Q_INVOKABLE void connect(const QString &url,const QString &username,const QString &password,int port);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE bool isConnected();
    Q_INVOKABLE QString sessionInfo();
    Q_INVOKABLE void sendPassword(const QString & password);
    Q_INVOKABLE void terminate();
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
