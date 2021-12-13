/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QPTYPROCESS_H
#define QPTYPROCESS_H

#include <QProcess>
#include <memory>

class KPtyDevice;

class QPtyProcess : public QProcess {
    Q_OBJECT

public:
    explicit QPtyProcess(QObject *parent = nullptr);
    QPtyProcess(int ptyMasterFd, QObject *parent = nullptr);
    ~QPtyProcess() override;
    bool atEnd() const override;
    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;
    bool canReadLine() const override;
    bool waitForBytesWritten(int msecs = 30000) override;
    bool waitForReadyRead(int msecs = 30000) override;
    qint64 pos() const override;
    bool reset() override;
    bool seek(qint64 pos) override;
    qint64 size() const override;
    bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite) override;

protected:
    void setupChildProcess() override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 readLineData(char *data, qint64 maxSize) override;

private:
    KPtyDevice * m_pty;
};

#endif
