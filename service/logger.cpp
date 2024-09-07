#include "logger.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

QFile * Logger::logfile = NULL;

const QString Logger::user_dir() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? NULL : getpwuid(uid));
    if (pw == NULL) return QString();

    return QString::fromLocal8Bit(pw->pw_dir);
}

const QString Logger::log_file_name() {
    QString userdir = user_dir();
    QString configdir = userdir + QDir::separator() +".config";
    if (!QDir(configdir).exists()) configdir = userdir;
    else {
        configdir += QDir::separator() + QCoreApplication::organizationName();
        QDir().mkpath(configdir);

    }
    return configdir + QDir::separator() + QCoreApplication::applicationName() + QLatin1String(".log");
}

QIODevice * Logger::device() {
    if (logfile != NULL) return logfile;
    logfile = new QFile(log_file_name());
    if (!logfile->open(QIODevice::WriteOnly)) logfile->open(2,QIODevice::WriteOnly);
    if (QCoreApplication::instance() != NULL) {
        QObject::connect(QCoreApplication::instance(),&QCoreApplication::aboutToQuit,logfile,&QObject::deleteLater);
    }
    return logfile;
}

Logger::Logger() : QDebug(device()) {}

Logger::~Logger() {
    if (logfile == NULL) return;
    *this << "\n";
    logfile->flush();
}
