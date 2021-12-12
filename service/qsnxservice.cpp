#include "qsnxservice.h"
#include "qsnx_adaptor.h"
#include <signal.h>
#include <QTemporaryFile>
#include <QFile>
#include <QDirIterator>
#include <QFileInfo>
#include <QThread>
#include <QDirIterator>
#include <QDebug>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>

const QList<int> x_displays() {
    QDirIterator iterator(QLatin1String(TMP_DIR)+QDir::separator()+QLatin1String(".X11-unix"),QDir::Files|QDir::System|QDir::NoDotDot|QDir::NoDot);
    QString name;
    int val;
    bool ok;
    QList<int> ret;
    while (iterator.hasNext()) {
        name = QFileInfo(iterator.next()).fileName();
        if (!name.startsWith('X')) continue;
        val = name.midRef(1).toInt(&ok);
        if (!ok) continue;
        ret.append(val);
    }
    return ret;
}

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

QString SNXProcess::errorString() const {
    return m_error.isEmpty()?m_process.errorString():m_error;
}

const QString SNXProcess::user_dir() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? NULL : getpwuid(uid));
    if (pw == NULL) return QString();

    return QString::fromLocal8Bit(pw->pw_dir);
}

void SNXProcess::init() {
    m_first_time = true;
    m_is_connected = false;
    m_process.setProgram(SNX_PATH);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString so_preload = env.value("LD_PRELOAD");
    QString so_bin;
    if (QFile(SO_BIN1).exists()) so_bin = SO_BIN1;
    else if (QFile(SO_BIN2).exists()) so_bin = SO_BIN2;
    if (!so_bin.isEmpty()) env.insert("LD_PRELOAD",QString("%2%1").arg(so_bin,so_preload.isEmpty()?"":(so_preload+":")));
    env.insert("SHELL",BASH_BIN);
    env.insert("HOME",user_dir());
    env.insert("TERM","xterm-256color");
    env.insert("USER","root");
    QList<int> xs = x_displays();
    if (!xs.isEmpty()) env.insert("DISPLAY",QString(":%1").arg(xs[0]));
    m_process.setProcessEnvironment(env);
    connect(&m_process,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),this,[=]() {
        if (!m_is_connected) emit disconnected();
        else {
            QThread::sleep(1);
            QMetaObject::invokeMethod(this,"forked",Qt::QueuedConnection);
        }
    });
    connect(&m_process,&QProcess::errorOccurred,(QSNXService *)parent(),[=]{ emit ((QSNXService *)parent())->error(m_process.errorString()); });
    connect(&m_process,&QProcess::readyReadStandardOutput,this,[=]() { while (m_process.canReadLine()) analyze_line(m_process.readLine()); });
    connect(&m_process,&QProcess::errorOccurred,this,[=](){ emit errorOccurred(m_process.errorString()); });
    connect(this,&SNXProcess::disconnected,this,[=](){ m_is_connected = false; m_connected_info.clear(); deleteLater(); });
    connect(this,&SNXProcess::connected,this,[=](){ m_is_connected = true; m_connected_info.clear(); });
}

void SNXProcess::forked() {
    qint64 pid;
    if ((pid = processId()) <= 0) {
        emit disconnected();
        return;
    }

    QTimer * timer = new QTimer(this);
    timer->setInterval(2000);
    timer->setProperty("filename",QDir::separator()+QLatin1String("proc")+QDir::separator()+QString("%1").arg(pid));
    connect(timer,&QTimer::timeout,this,[=]() {
        if (!QFileInfo::exists(timer->property("filename").toString())) emit disconnected();
    });
    timer->start();
}

void SNXProcess::terminate() {
    if (m_process.state() == QProcess::Running || m_process.state() == QProcess::Starting) {
        m_process.kill();
        return;
    }
    if (!m_is_connected) return;
    ::kill(SNXProcess::processId(SNX_PATH),SIGKILL);
}

qint64 SNXProcess::processId() const {
    if (m_process.state() == QProcess::Running) return (qint64)m_process.processId();
    if (!m_is_connected) return 0;
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
    return (m_is_connected && processId() > 0);
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
    if (m_process.state() == QProcess::NotRunning) return m_process.startDetached();
    return false;
}

QString SNXProcess::connnectedInfo() const {
    return m_connected_info;
}

void SNXProcess::start() {
    m_error.clear();
    if (!check_parameters()) return;
    if (m_is_connected) {
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
    else if (line == QLatin1String("SNX - connected.")) emit connected();
    else m_connected_info += line + QLatin1Char('\n');
    m_first_time = false;
}

QSNXService::QSNXService(QObject *parent) : QObject(parent) {
    new QSNXAdaptor(this);
    QDBusConnection dbus = QDBusConnection::systemBus();
    if (!dbus.registerObject("/", this)) {
        qDebug() << "Cannot register QSNXService object!" << dbus.lastError();
        exit(1);
    }
    if (!dbus.registerService("com.alexl.qt.QSNX")) {
        qDebug() << "Cannot register QSNXService service!" << dbus.lastError();
        exit(1);
    }

    m_process = NULL;
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
    QObject::connect(m_process,&SNXProcess::connected,this,&QSNXService::connected);
    QObject::connect(m_process,&SNXProcess::disconnected,this,&QSNXService::disconnected);
    QObject::connect(m_process,&SNXProcess::disconnected,this,[=]() { qDebug() << "disconnected"; m_process = NULL; });
    QObject::connect(m_process,&SNXProcess::errorOccurred,this,&QSNXService::error);
    m_process->start();
}

void QSNXService::disconnect() {
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
    if (m_process == NULL) return;
    m_process->terminate();
}
