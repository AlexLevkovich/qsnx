/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "qsnxservice.h"
#include "qsnx_adaptor.h"
#include <signal.h>
#include <QTemporaryFile>
#include <QFile>
#include <QDirIterator>
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>

const int SNXProcess::MAX_CONNECT_COUNT = 10;

SNXProcess::SNXProcess(QObject *parent) : QObject(parent) {
    init();
    m_process.setArguments(QStringList() << DISCONNECT_SWITCH);
}

SNXProcess::SNXProcess(const QString &url,const QString &certificate,int port,QObject *parent) : QObject(parent) {
    init();
    m_process.setArguments(QStringList() << URL_SWITCH << url << CERTIFICATE_SWITCH << certificate << PORT_SWITCH << QString("%1").arg(port));
    m_certificate = certificate;
}

SNXProcess::SNXProcess(const QString &url,const QString &username,const QString &password,int port,QObject *parent) : QObject(parent) {
    init();
    m_process.setArguments(QStringList() << URL_SWITCH << url << USER_SWITCH << username << PORT_SWITCH << QString("%1").arg(port));
    m_password = password;
    m_username = username;
}

SNXProcess::~SNXProcess() {}

QString SNXProcess::errorString() const {
    return m_error.isEmpty()?m_process.errorString():m_error;
}

const QString SNXProcess::user_dir() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? NULL : getpwuid(uid));
    if (pw == NULL) return QString();

    return QString::fromLocal8Bit(pw->pw_dir);
}

int SNXProcess::maxConnectCount() const {
    return m_max_connect_count;
}

void SNXProcess::setMaxConnectCount(int count) {
    m_max_connect_count = (count <= 0)?1:count;
}

void SNXProcess::init() {
    m_first_time = true;
    m_state = Stopped;
    m_connect_counter = 0;
    m_max_connect_count = MAX_CONNECT_COUNT;
    m_process.setProgram(SNX_PATH);
    connect(&m_process,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),this,[=]() {
        if (m_state == SortOfStarted) QTimer::singleShot(1000,this,[=]{ snx_forked(); });
        else emit disconnected();
    });
    connect(&m_process,&QProcess::started,this,&SNXProcess::connecting);
    connect(&m_process,&QProcess::errorOccurred,(QSNXService *)parent(),[=]{ emit ((QSNXService *)parent())->error(m_process.errorString()); });
    connect(&m_process,&QProcess::readyRead,this,[=]() { while (m_process.canReadLine()) analyze_line(m_process.readLine()); });
    connect(&m_process,&QProcess::errorOccurred,this,[=](){ emit errorOccurred(m_process.errorString()); });
    connect(this,&SNXProcess::disconnected,this,[=](){ m_connect_counter = 0; m_state = Stopped; dns_ips.clear(); dns_suffixes.clear(); m_connected_info.clear(); deleteLater(); });
    connect(this,&SNXProcess::connecting,this,[=](){ m_state = Starting; m_connected_info.clear(); });
    connect(this,&SNXProcess::connected,this,[=](){ m_state = Started; });
}

void SNXProcess::snx_forked() {
    qint64 pid;
    if ((pid = processId()) <= 0) {
        if (m_connect_counter < m_max_connect_count) {
            m_connect_counter++;
            m_state = Stopped;
            dns_ips.clear();
            dns_suffixes.clear();
            m_connected_info.clear();
            qDebug() << "SNX process exited on its own will, trying to reconnect...";
            QTimer::singleShot(1000,this,[=](){ start(); });
        }
        else {
            m_error = tr("SNX process exited on its own will");
            emit errorOccurred(m_error);
            emit disconnected();
        }
        return;
    }

    emit connected();
    QTimer * timer = new QTimer(this);
    timer->setInterval(2000);
    timer->setProperty("filename",QDir::separator()+QLatin1String("proc")+QDir::separator()+QString("%1").arg(pid));
    connect(timer,&QTimer::timeout,this,[=]() {
        if (!QFileInfo::exists(timer->property("filename").toString())) {
            timer->stop();
            emit disconnected();
            timer->deleteLater();
        }
    });
    timer->start();
}

void SNXProcess::terminate() {
    m_max_connect_count = 1;
    if (m_process.state() == QProcess::Running || m_process.state() == QProcess::Starting) {
        m_process.blockSignals(true);
        m_process.kill();
        if (m_process.state() == QProcess::Running) m_process.waitForFinished();
        emit disconnected();
        return;
    }
    if (m_state == Started) return;
    ::kill(SNXProcess::processId(SNX_PATH),SIGKILL);
}

qint64 SNXProcess::processId() const {
    if (m_process.state() == QProcess::Running) return (qint64)m_process.processId();
    if (m_state == Started) return 0;
    return SNXProcess::processId(SNX_PATH);
}

qint64 SNXProcess::processId(const QString & path) {
    if (!path.startsWith(QDir::separator())) return 0;
    QDirIterator dirs(QDir::separator()+QLatin1String("proc"),QDir::AllDirs|QDir::NoDotDot|QDir::NoDot);
    QString pid;
    QString exe_path;
    while (dirs.hasNext()) {
        pid = dirs.next();
        QFileInfo info(pid+QDir::separator()+QLatin1String("exe"));
        pid = pid.mid(6);
        if (!info.isSymLink()) continue;
        exe_path = info.symLinkTarget();
        if (exe_path.isEmpty()) continue;
        if (exe_path == path) return (qint64)pid.toLongLong();
    }
    return 0;
}

bool SNXProcess::isConnected() const {
    return ((m_state == Started) && processId() > 0);
}

bool SNXProcess::isRunning() const {
    return (processId() > 0);
}

bool SNXProcess::check_parameters() {
    m_error.clear();
    if (m_username.isEmpty() && m_certificate.isEmpty()) {
        m_error = tr("Username and certificate file path cannot be both empty!");
        QMetaObject::invokeMethod(this,"errorOccurred",Qt::QueuedConnection,Q_ARG(QString,m_error));
        return false;
    }
    return true;
}

bool SNXProcess::startDetached() {
    if (m_process.state() == QProcess::NotRunning) {
        bool ret = m_process.startDetached();
        if (ret) deleteLater();
        return ret;
    }
    return false;
}

QString SNXProcess::connnectedInfo() const {
    return m_connected_info;
}

void SNXProcess::start() {
    m_error.clear();
    if (!check_parameters()) return;
    if (m_state != Stopped) {
        m_error = tr("Cannot start the second process!");
        QMetaObject::invokeMethod(this,"errorOccurred",Qt::QueuedConnection,Q_ARG(QString,m_error));
        return;
    }
    if (m_process.state() == QProcess::NotRunning) m_process.start();
}

bool SNXProcess::write_data(const char * data) {
    qint64 bytes = 0;
    qint64 len = strlen(data);
    qint64 written;
    if (len <= 0) return true;
    do {
        written = m_process.write(data+bytes,len-bytes);
        if (written <= 0) return false;
        bytes += written;
    } while (bytes < len);
    return true;
}

void SNXProcess::sendPassword(const QString & password) {
    m_password = password;
    write_data((m_password.toLocal8Bit()+'\n').constData());
}

void SNXProcess::analyze_line(const QByteArray & array) {
    m_error.clear();
    QString line = QString::fromLatin1(array);
    if (line.endsWith(QLatin1Char('\n'))) line = line.left(line.length()-1);
    if (line.endsWith(QLatin1Char('\r'))) line = line.left(line.length()-1);
    qDebug() << line;
    if (m_first_time && line != QLatin1String("Check Point's Linux SNX") && line != QLatin1String("SNX - Disconnecting...")) {
        m_error = line;
        emit errorOccurred(m_error);
    }
    else if (line.startsWith("SNX: ")) {
        m_error = line.mid(5);
        emit errorOccurred(m_error);
    }
    else if (line.endsWith(QLatin1String(" [y]es/[N]o:"))) write_data("y");
    else if (line.endsWith(QLatin1String(" password:"))) {
        if (m_password.isEmpty()) emit passwordRequested();
        else sendPassword(m_password);
    }
    else if (line == QLatin1String("SNX - connected.")) m_state = SortOfStarted;
    else {
        if ((m_state == SortOfStarted) && (line.startsWith("DNS Server") || line.startsWith("Secondary DNS Server")) && line.contains(':')) dns_ips.append(line.split(':').at(1).trimmed());
        if ((m_state == SortOfStarted) && line.startsWith("DNS Suffix") && line.contains(':')) {
            dns_suffixes.clear();
            for (QString & suffix: line.split(':').at(1).trimmed().split(';',
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                                                         Qt::SkipEmptyParts
#else
                                                                         QString::SkipEmptyParts
#endif
                                                                         )) {
                dns_suffixes.append(suffix.trimmed());
            }
        }
        m_connected_info += line + QLatin1Char('\n');
    }
    m_first_time = false;
}

QStringList SNXProcess::dnsSuffixes() const {
    return dns_suffixes;
}

QStringList SNXProcess::dnsIPs() const {
    return dns_ips;
}

qint64 SNXProcess::startDetached(const QString & pgm,const QStringList & args) {
    QProcess proc;
    proc.setProgram(pgm);
    proc.setArguments(args);
    qint64 pid;
    if (!proc.startDetached(&pid)) return 0;
    return pid;
}

QSNXService::QSNXService(QObject *parent) : QObject(parent) {
    m_process = NULL;
    new QSNXAdaptor(this);
    QDBusConnection dbus = QDBusConnection::systemBus();
    if (!dbus.registerObject("/", this)) {
        qDebug() << "Cannot register QSNXService object!" << dbus.lastError();
        exit(1);
        return;
    }
    if (!dbus.registerService("com.alexl.qt.QSNX")) {
        qDebug() << "Cannot register QSNXService service!" << dbus.lastError();
        exit(1);
        return;
    }
}

QSNXService::~QSNXService() {
    disconnect();
}

void QSNXService::connect(const QString &url,const QString &certificate,int port) {
    start_process(new SNXProcess(url,certificate,port,this));
}

void QSNXService::connect(const QString &url,const QString &username,const QString &password,int port) {
    start_process(new SNXProcess(url,username,password,port,this));
}

void QSNXService::start_process(SNXProcess * process) {
    if (SNXProcess::processId(SNX_PATH) > 0) {
        emit error(SNX_PATH+tr(" is already running. Stop or kill it first."));
        return;
    }

    m_process = process;
    QObject::connect(m_process,&SNXProcess::passwordRequested,this,&QSNXService::passwordRequested);
    QObject::connect(m_process,&SNXProcess::connecting,this,&QSNXService::connecting);
    QObject::connect(m_process,&SNXProcess::connected,this,&QSNXService::connected);
    QObject::connect(m_process,&SNXProcess::connected,this,[=]() {
        if (SNXProcess::processId(SYSTEMD_RESOLVED) <= 0 || !QFile(SYSTEMD_RESOLVE).exists()) return;
        qDebug() << "using systemd_resolved for dns...";
        QStringList args;
        args << RESOLVE_IF_SWITCH << TUNIF;
        for (QString & ip: m_process->dnsIPs()) {
            args << RESOLVE_DNS_SWITCH << ip;
        }
        for (QString & suffix: m_process->dnsSuffixes()) {
            args << RESOLVE_DOMAIN_SWITCH << suffix;
        }
        qDebug() << SYSTEMD_RESOLVE << args;
        SNXProcess::startDetached(SYSTEMD_RESOLVE,args);
    });
    QObject::connect(m_process,&SNXProcess::disconnected,this,&QSNXService::disconnected);
    QObject::connect(m_process,&SNXProcess::disconnected,this,[=]() {
        qDebug() << "disconnected";
        m_process = NULL;
    });
    QObject::connect(m_process,&SNXProcess::errorOccurred,this,&QSNXService::error);
    m_process->start();
}

void QSNXService::disconnect() {
    qDebug() << "disconnecting by client...";
    if (m_process != NULL) m_process->setMaxConnectCount(1);
    (new SNXProcess(this))->startDetached();
}

bool QSNXService::isConnected() {
    return (m_process != NULL && m_process->isConnected());
}

QString QSNXService::sessionInfo() {
    if (m_process == NULL) return QString();
    return m_process->connnectedInfo();
}

void QSNXService::sendPassword(const QString & password) {
    if (m_process == NULL) return;
    m_process->sendPassword(password);
}

void QSNXService::terminate() {
    qDebug() << "terminating by client...";
    if (m_process == NULL) return;
    m_process->terminate();
}
