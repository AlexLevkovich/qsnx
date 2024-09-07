#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>

class QFile;

class Logger : public QDebug {
public:
    Logger();
    ~Logger();
private:
    static QIODevice * device();
    static const QString user_dir();
    static const QString log_file_name();
    static QFile * logfile;
};

#endif // LOGGER_H
