#include "spdLog.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"


namespace util {

    spdlog::logger* LOGGER = nullptr;

#define WDZ_DEBUG(...) SPDLOG_LOGGER_CALL(LOGGER, spdlog::level::debug, __VA_ARGS__)
#define WDZ_INFO(...) SPDLOG_LOGGER_CALL(LOGGER, spdlog::level::info, __VA_ARGS__)
#define WDZ_WARN(...) SPDLOG_LOGGER_CALL(LOGGER, spdlog::level::warn, __VA_ARGS__)
#define WDZ_ERROR(...) SPDLOG_LOGGER_CALL(LOGGER, spdlog::level::err, __VA_ARGS__)
#define WDZ_CRITICAL(...) SPDLOG_LOGGER_CALL(LOGGER, spdlog::level::critical, __VA_ARGS__)
    //////////////////////////////////////////////////////////////////////////

    void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
    {
        switch (type)
        {
        case QtDebugMsg:
            LOGGER->log(spdlog::source_loc{ context.file, context.line, SPDLOG_FUNCTION }, spdlog::level::debug, msg.toLocal8Bit().data());
            break;
        case QtWarningMsg:
            LOGGER->log(spdlog::source_loc{ context.file, context.line, SPDLOG_FUNCTION }, spdlog::level::warn, msg.toLocal8Bit().data());
            break;
        case QtCriticalMsg:
            LOGGER->log(spdlog::source_loc{ context.file, context.line, SPDLOG_FUNCTION }, spdlog::level::critical, msg.toLocal8Bit().data());
            break;
        case QtFatalMsg:
            LOGGER->log(spdlog::source_loc{ context.file, context.line, SPDLOG_FUNCTION }, spdlog::level::err, msg.toLocal8Bit().data());
            break;
        case QtInfoMsg:
            LOGGER->log(spdlog::source_loc{ context.file, context.line, SPDLOG_FUNCTION }, spdlog::level::info, msg.toLocal8Bit().data());
            break;
        default:
            break;
        }
    }

    void initSpdlog(QString logPath)
    {
        //初始化spdlog日志库
        spdlog::set_level(spdlog::level::trace);
#ifdef Q_OS_WIN
        auto consle_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else
        auto consle_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif // Q_OS_WIN
        auto daily_file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logPath.isEmpty() ? "logs/log.txt" : logPath.toLocal8Bit().data(), 0,0, false, 2);
        LOGGER = new spdlog::logger("multi_sink", { consle_sink,daily_file_sink });
        LOGGER->set_level(spdlog::level::trace);
        LOGGER->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^%s:%# [%l][%t] %v%$");
        LOGGER->flush_on(spdlog::level::debug);
    }
}