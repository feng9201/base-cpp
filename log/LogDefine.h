#ifndef _LOGDEFINE
#define _LOGDEFINE
//#include <log4qt/Log4Qt>
//#include <log4qt/PropertyConfigurator>
//#include <log4qt/LogManager>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QString>

// 以文件行列记录信息，非以类名对象形式
#define LOG_DEBUG(msg) qDebug() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%2:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("DEBUG")  \
                                           <<(msg);

#define LOG_INFO(msg) qInfo() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%2:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("INFO")  \
                                           <<(msg);

#define LOG_WARN(msg) qWarning() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%2:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("WARN")  \
                                           <<(msg);

#define LOG_ERROR(msg) qFatal() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%2:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("ERROR")  \
                                           <<(msg);


// 以类名对象形式记录信息
#define LOG_OBJECT_DEBUG(msg) qDebug() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%3").arg(__FUNCTION__).arg(__LINE__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("DEBUG")  \
                                           <<(msg);

#define LOG_OBJECT_INFO(msg) qInfo() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%3").arg(__FUNCTION__).arg(__LINE__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("INFO")  \
                                           <<(msg);

#define LOG_OBJECT_WARN(msg) qWarning() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%3").arg(__FUNCTION__).arg(__LINE__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("WARN")  \
                                           <<(msg);

#define LOG_OBJECT_ERROR(msg) qFatal() << QString("[%1][%2][%3][%4]")  \
                                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                           .arg(QString("%1:%3").arg(__FUNCTION__).arg(__LINE__))  \
                                           .arg(QThread::currentThread()->objectName())  \
                                           .arg("ERROR")  \
                                           <<(msg);


#endif