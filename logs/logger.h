﻿#ifndef MTAO_LOGGER_H_
#define MTAO_LOGGER_H_
/*********
@brief 日志记录类，支持控制台&文件输出
@data  2025-03-26
@use
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    auto logpath = "E:/work-tmp/MtBase/bin/Debug/logs";
    auto file_ptr = std::make_shared<FileChannel>(logpath);
    Logger::Instance().add(file_ptr);
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());
    DebugL << "this is a repeat 2 times log";
    std::string cc = "这是个啥";
    PrintT("this is a %s test:%d", cc.c_str(), 124);
********/

#include <cstdarg>
#include <set>
#include <map>
#include <fstream>
#include <thread>
#include <memory>
#include <iostream>
#include <sstream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <functional>

#if defined(_WIN32)
#undef FD_SETSIZE
#define FD_SETSIZE 1024
#include <winsock2.h>
#pragma comment (lib,"WS2_32")
#else
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cstddef>
#endif // defined(_WIN32)



#if defined(_WIN32)
struct dirent {
    long d_ino;              /* inode number*/
    off_t d_off;             /* offset to this dirent*/
    unsigned short d_reclen; /* length of this d_name*/
    unsigned char d_type;    /* the type of d_name*/
    char d_name[1];          /* file name (null-terminated)*/
};
typedef struct _dirdesc {
    int     dd_fd;      /** file descriptor associated with directory */
    long    dd_loc;     /** offset in current buffer */
    long    dd_size;    /** amount of data returned by getdirentries */
    char* dd_buf;    /** data buffer */
    int     dd_len;     /** size of data buffer */
    long    dd_seek;    /** magic cookie returned by getdirentries */
    HANDLE handle;
    struct dirent* index;
} DIR;
# define __dirfd(dp)    ((dp)->dd_fd)

int mkdir(const char* path, int mode);
DIR* opendir(const char*);
int closedir(DIR*);
struct dirent* readdir(DIR*);

#endif // defined(_WIN32)

#if defined(_WIN32) || defined(_WIN64)
#define fseek64 _fseeki64
#define ftell64 _ftelli64
#else
#define fseek64 fseek
#define ftell64 ftell
#endif

namespace mttool {
#ifndef _WIN32
    void local_time_init();
    void no_locks_localtime(struct tm *tmp, time_t t);
    struct tm getLocalTime(time_t sec);
#endif

/******************************信号量封装****************************************/
    class semaphore {
    public:
        explicit semaphore(size_t initial = 0) {
#if defined(HAVE_SEM)
            sem_init(&_sem, 0, initial);
#else
            _count = 0;
#endif
        }
        ~semaphore() {
#if defined(HAVE_SEM)
            sem_destroy(&_sem);
#endif
        }
        void post(size_t n = 1) {
#if defined(HAVE_SEM)
            while (n--) {
                sem_post(&_sem);
            }
#else
            std::unique_lock<std::recursive_mutex> lock(_mutex);
            _count += n;
            if (n == 1) {
                _condition.notify_one();
            }
            else {
                _condition.notify_all();
            }
#endif
        }

        void wait() {
#if defined(HAVE_SEM)
            sem_wait(&_sem);
#else
            std::unique_lock<std::recursive_mutex> lock(_mutex);
            while (_count == 0) {
                _condition.wait(lock);
            }
            --_count;
#endif
        }

    private:
#if defined(HAVE_SEM)
        sem_t _sem;
#else
        size_t _count;
        std::recursive_mutex _mutex;
        std::condition_variable_any _condition;
#endif
    };

/******************************文件操作封装****************************************/
class File {
public:
    static bool create_path(const std::string& file, unsigned int mod);

    static void scanDir(const std::string& path, const std::function<bool(const std::string& path, bool isDir)>& cb,
        bool enter_subdirectory = false, bool show_hidden_file = false);

    static bool is_special_dir(const std::string& path);

    static bool is_dir(const std::string& path);

    //删除目录或文件  [AUTO-TRANSLATED:79bed783]
    //Delete a directory or file
    static int delete_file(const std::string& path, bool del_empty_dir = false, bool backtrace = true);

    static void deleteEmptyDir(const std::string& dir, bool backtrace);

    static std::string parentDir(const std::string& path);

private:
    File();
    ~File();
};

/******************************禁止拷贝基类****************************************/
class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    //禁止拷贝  [AUTO-TRANSLATED:e8af72e3]
    //Prohibit copying
    noncopyable(const noncopyable& that) = delete;
    noncopyable(noncopyable&& that) = delete;
    noncopyable& operator=(const noncopyable& that) = delete;
    noncopyable& operator=(noncopyable&& that) = delete;
};

#define INSTANCE_IMP(class_name, ...) \
class_name &class_name::Instance() { \
    static std::shared_ptr<class_name> s_instance(new class_name(__VA_ARGS__)); \
    static class_name &s_instance_ref = *s_instance; \
    return s_instance_ref; \
}

class LogContext;
class LogChannel;
class LogWriter;
class Logger;

using LogContextPtr = std::shared_ptr<LogContext>;

typedef enum {
    LTrace = 0, LDebug, LInfo, LWarn, LError
} LogLevel;

Logger &getLogger();
void setLogger(Logger *logger);


/**
* 日志类
 * Log class
 
 * [AUTO-TRANSLATED:3f74af09]
*/
class Logger : public std::enable_shared_from_this<Logger>, public noncopyable {
public:
    friend class AsyncLogWriter;
    using Ptr = std::shared_ptr<Logger>;

    /**
     * 获取日志单例
     * @return
     * Get log singleton
     * @return
     
     * [AUTO-TRANSLATED:9f0d1ed7]
     */
    static Logger &Instance();

    explicit Logger(const std::string &loggerName);
    ~Logger();

    /**
     * 添加日志通道，非线程安全的
     * @param channel log通道
     * Add log channel, not thread-safe
     * @param channel log channel
     
     * [AUTO-TRANSLATED:4c801098]
     */
    void add(const std::shared_ptr<LogChannel> &channel);

    /**
     * 删除日志通道，非线程安全的
     * @param name log通道名
     * Delete log channel, not thread-safe
     * @param name log channel name
     
     * [AUTO-TRANSLATED:fa78e18b]
     */
    void del(const std::string &name);

    /**
     * 获取日志通道，非线程安全的
     * @param name log通道名
     * @return 线程通道
     * Get log channel, not thread-safe
     * @param name log channel name
     * @return log channel
     
     * [AUTO-TRANSLATED:4fa3f6c3]
     */
    std::shared_ptr<LogChannel> get(const std::string &name);

    /**
     * 设置写log器，非线程安全的
     * @param writer 写log器
     * Set log writer, not thread-safe
     * @param writer log writer
     
     * [AUTO-TRANSLATED:da1403f7]
     */
    void setWriter(const std::shared_ptr<LogWriter> &writer);

    /**
     * 设置所有日志通道的log等级
     * @param level log等级
     * Set log level for all log channels
     * @param level log level
     
     * [AUTO-TRANSLATED:d064460c]
     */
    void setLevel(LogLevel level);

    /**
     * 获取logger名
     * @return logger名
     * Get logger name
     * @return logger name
     
     * [AUTO-TRANSLATED:e0de492b]
     */
    const std::string &getName() const;

    /**
     * 写日志
     * @param ctx 日志信息
     * Write log
     * @param ctx log information
     
     * [AUTO-TRANSLATED:4f29cde6]
     */
    void write(const LogContextPtr &ctx);

private:
    /**
     * 写日志到各channel，仅供AsyncLogWriter调用
     * @param ctx 日志信息
     * Write log to each channel, only for AsyncLogWriter to call
     * @param ctx log information
     
     * [AUTO-TRANSLATED:caad57f4]
     */
    void writeChannels(const LogContextPtr &ctx);
    void writeChannels_l(const LogContextPtr &ctx);

private:
    LogContextPtr _last_log;
    std::string _logger_name;
    std::shared_ptr<LogWriter> _writer;
    std::shared_ptr<LogChannel> _default_channel;
    std::map<std::string, std::shared_ptr<LogChannel> > _channels;
};

///////////////////LogContext///////////////////
/**
* 日志上下文
 * Log Context
 
 * [AUTO-TRANSLATED:f2805fe8]
*/
class LogContext : public std::ostringstream {
public:
    //_file,_function改成string保存，目的是有些情况下，指针可能会失效  [AUTO-TRANSLATED:8e4b3f48]
    //_file,_function changed to string to save, the purpose is that in some cases, the pointer may become invalid
    //比如说动态库中打印了一条日志，然后动态库卸载了，那么指向静态数据区的指针就会失效  [AUTO-TRANSLATED:d5e087bc]
    //For example, a log is printed in a dynamic library, and then the dynamic library is unloaded, so the pointer to the static data area will become invalid
    LogContext() = default;
    LogContext(LogLevel level, const char *file, const char *function, int line, const char *module_name, const char *flag);
    ~LogContext() = default;

    LogLevel _level;
    int _line;
    int _repeat = 0;
    std::string _file;
    std::string _function;
    std::string _thread_name;
    std::string _module_name;
    std::string _flag;
    struct timeval _tv;

    const std::string &str();

private:
    bool _got_content = false;
    std::string _content;
};

/**
 * 日志上下文捕获器
 * Log Context Capturer
 
 * [AUTO-TRANSLATED:3c5ddc22]
 */
class LogContextCapture {
public:
    using Ptr = std::shared_ptr<LogContextCapture>;

    LogContextCapture(Logger &logger, LogLevel level, const char *file, const char *function, int line, const char *flag = "");
    LogContextCapture(const LogContextCapture &that);
    ~LogContextCapture();

    /**
     * 输入std::endl(回车符)立即输出日志
     * @param f std::endl(回车符)
     * @return 自身引用
     * Input std::endl (newline character) to output log immediately
     * @param f std::endl (newline character)
     * @return Self-reference
     
     * [AUTO-TRANSLATED:019b9eea]
     */
    LogContextCapture &operator<<(std::ostream &(*f)(std::ostream &));

    template<typename T>
    LogContextCapture &operator<<(T &&data) {
        if (!_ctx) {
            return *this;
        }
        (*_ctx) << std::forward<T>(data);
        return *this;
    }

    void clear();

private:
    LogContextPtr _ctx;
    Logger &_logger;
};


///////////////////LogWriter///////////////////
/**
 * 写日志器
 * Log Writer
 
 * [AUTO-TRANSLATED:f70397d4]
 */
class LogWriter : public noncopyable {
public:
    LogWriter() = default;
    virtual ~LogWriter() = default;

    virtual void write(const LogContextPtr &ctx, Logger &logger) = 0;
};

class AsyncLogWriter : public LogWriter {
public:
    AsyncLogWriter();
    ~AsyncLogWriter();

private:
    void run();
    void flushAll();
    void write(const LogContextPtr &ctx, Logger &logger) override;

private:
    bool _exit_flag;
    semaphore _sem;
    std::mutex _mutex;
    std::shared_ptr<std::thread> _thread;
    //List<std::pair<LogContextPtr, Logger *> > _pending;
    std::list<std::pair<LogContextPtr, Logger*> > _pending;
};

///////////////////LogChannel///////////////////
/**
 * 日志通道
 * Log Channel
 
 * [AUTO-TRANSLATED:afbe7d5f]
 */
class LogChannel : public noncopyable {
public:
    LogChannel(const std::string &name, LogLevel level = LTrace);
    virtual ~LogChannel();

    virtual void write(const Logger &logger, const LogContextPtr &ctx) = 0;
    const std::string &name() const;
    void setLevel(LogLevel level);
    static std::string printTime(const timeval &tv);

protected:
    /**
    * 打印日志至输出流
    * @param ost 输出流
    * @param enable_color 是否启用颜色
    * @param enable_detail 是否打印细节(函数名、源码文件名、源码行)
     * Print log to output stream
     * @param ost Output stream
     * @param enable_color Whether to enable color
     * @param enable_detail Whether to print details (function name, source file name, source line)
     
     * [AUTO-TRANSLATED:54c78737]
    */
    virtual void format(const Logger &logger, std::ostream &ost, const LogContextPtr &ctx, bool enable_color = true, bool enable_detail = true);

protected:
    std::string _name;
    LogLevel _level;
};

///**
// * 输出日至到广播
// * Output log to broadcast
// 
// * [AUTO-TRANSLATED:ee99643f]
// */
//class EventChannel : public LogChannel {
//public:
//    //输出日志时的广播名  [AUTO-TRANSLATED:2214541b]
//    //Broadcast name when outputting log
//    static const std::string kBroadcastLogEvent;
//    //mttool目前仅只有一处全局变量被外部引用，减少导出相关定义，导出以下函数避免导出kBroadcastLogEvent  [AUTO-TRANSLATED:71271efd]
//    //The mttool currently only has one global variable referenced externally, reducing the export of related definitions, and exporting the following functions to avoid exporting kBroadcastLogEvent
//    static const std::string& getBroadcastLogEventName();
//    //日志广播参数类型和列表  [AUTO-TRANSLATED:20255585]
//    //Log broadcast parameter type and list
//    #define BroadcastLogEventArgs const Logger &logger, const LogContextPtr &ctx
//
//    EventChannel(const std::string &name = "EventChannel", LogLevel level = LTrace);
//    ~EventChannel() override = default;
//
//    void write(const Logger &logger, const LogContextPtr &ctx) override;
//};

/**
 * 输出日志至终端，支持输出日志至android logcat
 * Output logs to the terminal, supporting output to Android logcat
 
 * [AUTO-TRANSLATED:538b78dc]
 */
class ConsoleChannel : public LogChannel {
public:
    ConsoleChannel(const std::string &name = "ConsoleChannel", LogLevel level = LTrace);
    ~ConsoleChannel() override = default;

    void write(const Logger &logger, const LogContextPtr &logContext) override;
};

/**
 * 输出日志至文件
 * Output logs to a file
 
 * [AUTO-TRANSLATED:c905542e]
 */
class FileChannelBase : public LogChannel {
public:
    FileChannelBase(const std::string &name = "FileChannelBase", const std::string &path = "./tmp/log", LogLevel level = LTrace);
    ~FileChannelBase() override;

    void write(const Logger &logger, const LogContextPtr &ctx) override;
    bool setPath(const std::string &path);
    const std::string &path() const;

protected:
    virtual bool open();
    virtual void close();
    virtual size_t size();

protected:
    std::string _path;
    std::ofstream _fstream;
};

class Ticker;

/**
 * 自动清理的日志文件通道
 * 默认最多保存30天的日志
 * Auto-cleaning log file channel
 * Default to keep logs for up to 30 days
 
 * [AUTO-TRANSLATED:700cb04b]
 */
class FileChannel : public FileChannelBase {
public:
    FileChannel( const std::string &dir = "./tmp/log", const std::string& name = "FileChannel", LogLevel level = LTrace);
    ~FileChannel() override = default;

    /**
     * 写日志时才会触发新建日志文件或者删除老的日志文件
     * @param logger
     * @param stream
     * Trigger new log file creation or deletion of old log files when writing logs
     * @param logger
     * @param stream
     
     * [AUTO-TRANSLATED:b8e3a717]
     */
    void write(const Logger &logger, const LogContextPtr &ctx) override;

    /**
     * 设置日志最大保存天数
     * @param max_day 天数
     * Set the maximum number of days to keep logs
     * @param max_day Number of days
     
     * [AUTO-TRANSLATED:317426b9]
     */
    void setMaxDay(size_t max_day);

    /**
     * 设置日志切片文件最大大小
     * @param max_size 单位MB
     * Set the maximum size of log slice files
     * @param max_size Unit: MB
     
     * [AUTO-TRANSLATED:071a8ec2]
     */
    void setFileMaxSize(size_t max_size);

    /**
     * 设置日志切片文件最大个数
     * @param max_count 个数
     * Set the maximum number of log slice files
     * @param max_count Number of files
     
     * [AUTO-TRANSLATED:74da4e7f]
     */
    void setFileMaxCount(size_t max_count);

private:
    /**
     * 删除日志切片文件，条件为超过最大保存天数与最大切片个数
     * Delete log slice files, conditions are exceeding the maximum number of days and slices
     
     * [AUTO-TRANSLATED:9ddfccec]
     */
    void clean();

    /**
     * 检查当前日志切片文件大小，如果超过限制，则创建新的日志切片文件
     * Check the current log slice file size, if exceeded the limit, create a new log slice file
     
     * [AUTO-TRANSLATED:cfb08734]
     */
    void checkSize(time_t second);

    /**
     * 创建并切换到下一个日志切片文件
     * Create and switch to the next log slice file
     
     * [AUTO-TRANSLATED:dc55a521]
     */
    void changeFile(time_t second);

private:
    bool _can_write = false;
    //默认最多保存30天的日志文件  [AUTO-TRANSLATED:f16d4661]
    //Default to keep log files for up to 30 days
    size_t _log_max_day = 30;
    //每个日志切片文件最大默认128MB  [AUTO-TRANSLATED:6d3efa5e]
    //Maximum default size of each log slice file is 128MB
    size_t _log_max_size = 128;
    //最多默认保持30个日志切片文件  [AUTO-TRANSLATED:90689f74]
    //Default to keep up to 30 log slice files
    size_t _log_max_count = 30;
    //当前日志切片文件索引  [AUTO-TRANSLATED:a9894a48]
    //Current log slice file index
    size_t _index = 0;
    int64_t _last_day = -1;
    time_t _last_check_time = 0;
    std::string _dir;
    std::set<std::string> _log_file_map;
};

#if defined(__MACH__) || ((defined(__linux) || defined(__linux__)) && !defined(ANDROID))
class SysLogChannel : public LogChannel {
public:
    SysLogChannel(const std::string &name = "SysLogChannel", LogLevel level = LTrace);
    ~SysLogChannel() override = default;

    void write(const Logger &logger, const LogContextPtr &logContext) override;
};

#endif//#if defined(__MACH__) || ((defined(__linux) || defined(__linux__)) &&  !defined(ANDROID))

class BaseLogFlagInterface {
protected:
    virtual ~BaseLogFlagInterface() {}
    // 获得日志标记Flag  [AUTO-TRANSLATED:a8326285]
    //Get log flag
    const char* getLogFlag(){
        return _log_flag;
    }
    void setLogFlag(const char *flag) { _log_flag = flag; }
private:
    const char *_log_flag = "";
};

class LoggerWrapper {
public:
    template<typename First, typename ...ARGS>
    static inline void printLogArray(Logger &logger, LogLevel level, const char *file, const char *function, int line, First &&first, ARGS &&...args) {
        LogContextCapture log(logger, level, file, function, line);
        log << std::forward<First>(first);
        appendLog(log, std::forward<ARGS>(args)...);
    }

    static inline void printLogArray(Logger &logger, LogLevel level, const char *file, const char *function, int line) {
        LogContextCapture log(logger, level, file, function, line);
    }

    template<typename Log, typename First, typename ...ARGS>
    static inline void appendLog(Log &out, First &&first, ARGS &&...args) {
        out << std::forward<First>(first);
        appendLog(out, std::forward<ARGS>(args)...);
    }

    template<typename Log>
    static inline void appendLog(Log &out) {}

    //printf样式的日志打印  [AUTO-TRANSLATED:4a52190b]
    //printf style log print
    static void printLog(Logger &logger, int level, const char *file, const char *function, int line, const char *fmt, ...);
    static void printLogV(Logger &logger, int level, const char *file, const char *function, int line, const char *fmt, va_list ap);
};

//可重置默认值  [AUTO-TRANSLATED:b1e0e8b9]
//Can reset default value
extern Logger *g_defaultLogger;

//用法: DebugL << 1 << "+" << 2 << '=' << 3;  [AUTO-TRANSLATED:e6efe6cb]
//Usage: DebugL << 1 << "+" << 2 << '=' << 3;
#define WriteL(level) ::mttool::LogContextCapture(::mttool::getLogger(), level, __FILE__, __FUNCTION__, __LINE__)
#define TraceL WriteL(::mttool::LTrace)
#define DebugL WriteL(::mttool::LDebug)
#define InfoL WriteL(::mttool::LInfo)
#define WarnL WriteL(::mttool::LWarn)
#define ErrorL WriteL(::mttool::LError)

//只能在虚继承BaseLogFlagInterface的类中使用  [AUTO-TRANSLATED:a395e54d]
//Can only be used in classes that virtually inherit from BaseLogFlagInterface
#define WriteF(level) ::mttool::LogContextCapture(::mttool::getLogger(), level, __FILE__, __FUNCTION__, __LINE__, getLogFlag())
#define TraceF WriteF(::mttool::LTrace)
#define DebugF WriteF(::mttool::LDebug)
#define InfoF WriteF(::mttool::LInfo)
#define WarnF WriteF(::mttool::LWarn)
#define ErrorF WriteF(::mttool::LError)

//用法: PrintD("%d + %s = %c", 1 "2", 'c');  [AUTO-TRANSLATED:1217cc82]
//Usage: PrintD("%d + %s = %c", 1, "2", 'c');
#define PrintLog(level, ...) ::mttool::LoggerWrapper::printLog(::mttool::getLogger(), level, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define PrintT(...) PrintLog(::mttool::LTrace, ##__VA_ARGS__)
#define PrintD(...) PrintLog(::mttool::LDebug, ##__VA_ARGS__)
#define PrintI(...) PrintLog(::mttool::LInfo, ##__VA_ARGS__)
#define PrintW(...) PrintLog(::mttool::LWarn, ##__VA_ARGS__)
#define PrintE(...) PrintLog(::mttool::LError, ##__VA_ARGS__)

//用法: LogD(1, "+", "2", '=', 3);  [AUTO-TRANSLATED:2a824fae]
//Usage: LogD(1, "+", "2", '=', 3);
//用于模板实例化的原因，如果每次打印参数个数和类型不一致，可能会导致二进制代码膨胀  [AUTO-TRANSLATED:c40b3f4e]
//Used for template instantiation, because if the number and type of print parameters are inconsistent each time, it may cause binary code bloat
#define LogL(level, ...) ::mttool::LoggerWrapper::printLogArray(::mttool::getLogger(), (LogLevel)level, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LogT(...) LogL(::mttool::LTrace, ##__VA_ARGS__)
#define LogD(...) LogL(::mttool::LDebug, ##__VA_ARGS__)
#define LogI(...) LogL(::mttool::LInfo, ##__VA_ARGS__)
#define LogW(...) LogL(::mttool::LWarn, ##__VA_ARGS__)
#define LogE(...) LogL(::mttool::LError, ##__VA_ARGS__)

} /* namespace mttool */
#endif /* MTAO_LOGGER_H_ */
