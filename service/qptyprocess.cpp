/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "qptyprocess.h"
#include <unistd.h>
#include <pwd.h>
#include <kptydevice.h>
#include <stdlib.h>
#include <unistd.h>

const QString user_name() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? NULL : getpwuid(uid));
    if (pw == NULL) return QString();

    return QString::fromLocal8Bit(pw->pw_name);
}

QPtyProcess::QPtyProcess(QObject *parent) : QPtyProcess(-1, parent) {}

QPtyProcess::QPtyProcess(int ptyMasterFd, QObject *parent) : QProcess(parent) {
    m_pty = new KPtyDevice(this);

    if (ptyMasterFd == -1) {
        m_pty->open();
    } else {
        m_pty->open(ptyMasterFd);
    }

    connect(this,&QProcess::stateChanged,this,[this](QProcess::ProcessState state) {
        if (state == QProcess::NotRunning) m_pty->logout();
    });
    connect(m_pty,&KPtyDevice::readyRead,this,&QPtyProcess::readyRead);
    connect(m_pty,&KPtyDevice::readyRead,this,[=]() {
        QMetaObject::invokeMethod(this,"readyReadStandardOutput",Qt::QueuedConnection);
    });
    connect(m_pty,&KPtyDevice::bytesWritten,this,&QPtyProcess::bytesWritten);
}

QPtyProcess::~QPtyProcess() {
    if (state() != QProcess::NotRunning) m_pty->logout();
}

void QPtyProcess::setupChildProcess() {
    m_pty->setCTty();
    m_pty->login(user_name().toLocal8Bit().constData(),qgetenv("DISPLAY").constData());
    dup2(m_pty->slaveFd(),0);
    dup2(m_pty->slaveFd(),1);
    dup2(m_pty->slaveFd(),2);

    QProcess::setupChildProcess();
}

bool QPtyProcess::atEnd() const {
    return m_pty->atEnd();
}

qint64 QPtyProcess::bytesAvailable() const {
    return m_pty->bytesAvailable();
}

qint64 QPtyProcess::bytesToWrite() const {
    return m_pty->bytesToWrite();
}

bool QPtyProcess::canReadLine() const {
    return m_pty->canReadLine();
}

bool QPtyProcess::waitForBytesWritten(int msecs) {
    bool ret = m_pty->waitForBytesWritten(msecs);
    if (!ret && !m_pty->errorString().isEmpty()) {
        setErrorString(m_pty->errorString());
        emit errorOccurred(QProcess::Timedout);
    }
    return ret;
}

bool QPtyProcess::waitForReadyRead(int msecs) {
    bool ret = m_pty->waitForReadyRead(msecs);
    if (!ret && !m_pty->errorString().isEmpty()) {
        setErrorString(m_pty->errorString());
        emit errorOccurred(QProcess::Timedout);
    }
    return ret;
}

qint64 QPtyProcess::readData(char *data, qint64 maxlen) {
    qint64 ret = m_pty->read(data,maxlen);
    if (ret < 0) {
        setErrorString(m_pty->errorString());
        emit errorOccurred(QProcess::ReadError);
    }
    return ret;
}

qint64 QPtyProcess::readLineData(char *data, qint64 maxSize) {
    qint64 ret = m_pty->readLine(data,maxSize);
    if (ret < 0) {
        setErrorString(m_pty->errorString());
        emit errorOccurred(QProcess::ReadError);
    }
    return ret;
}

qint64 QPtyProcess::writeData(const char *data, qint64 len) {
    qint64 ret = m_pty->write(data,len);
    if (ret < 0) {
        setErrorString(m_pty->errorString());
        emit errorOccurred(QProcess::WriteError);
    }
    return ret;
}

qint64 QPtyProcess::pos() const {
    return m_pty->pos();
}

bool QPtyProcess::reset() {
    return m_pty->reset();
}

bool QPtyProcess::seek(qint64 pos) {
    bool ret = m_pty->seek(pos);
    if (!ret && !m_pty->errorString().isEmpty()) {
        setErrorString(m_pty->errorString());
        emit errorOccurred(QProcess::ReadError);
    }
    return ret;
}

qint64 QPtyProcess::size() const {
    return m_pty->size();
}

bool QPtyProcess::open(QIODevice::OpenMode) {
    return QProcess::open(QIODevice::ReadWrite);
}
