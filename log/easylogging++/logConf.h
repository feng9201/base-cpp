#pragma once

#include "easylogging++.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>


INITIALIZE_EASYLOGGINGPP
namespace easyLog {

	static unsigned int idx;
	void rolloutHandler(const char* filename, std::size_t size)
	{
		/// 备份日志
		QFile file(filename);
		QFileInfo fileInfo(file); // 获取原始文件信息
		QString newFilePath = fileInfo.path() + "/" + fileInfo.baseName() + "_" + QString::number(idx) + ".log"; // 拼接新的文件路径
		if (idx > 2) {
			idx = 0;
		}
		QFile newfile(newFilePath);
		if (newfile.exists()) {
			newfile.remove();
		}
		if (!file.rename(newFilePath)) { // 重命名文件
			int i = 0;
		}
		else {
			idx++;
		}
	}

	void	installLog()
	{
		el::Configurations conf;
		// 启用日志
		conf.setGlobally(el::ConfigurationType::Enabled, "true");
		//设置日志文件目录以及文件名
		QString path = QCoreApplication::applicationDirPath();
		path = path.append("\\log\\log_%datetime{%Y%M%d_%H%m%s}.log");
		conf.setGlobally(el::ConfigurationType::Filename, path.toStdString());
		//设置日志文件最大文件大小 10M
		conf.setGlobally(el::ConfigurationType::MaxLogFileSize, "10485760");
		//是否写入文件
		conf.setGlobally(el::ConfigurationType::ToFile, "true");
		//是否输出控制台
		conf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
		//设置日志输出格式
		conf.setGlobally(el::ConfigurationType::Format, "[%datetime] [%loc] [%level] : %msg");
		//设置日志文件写入周期，如下每100条刷新到输出流中
		conf.setGlobally(el::ConfigurationType::LogFlushThreshold, "10");
		//设置配置文件
		el::Loggers::reconfigureAllLoggers(conf);


		el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
		/// 防止Fatal级别日志中断程序
		el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
		/// 选择划分级别的日志	
		el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
		/// 设置级别门阀值，修改参数可以控制日志输出
		el::Loggers::setLoggingLevel(el::Level::Fatal);
		// 日志最大存储回调
		el::Helpers::installPreRollOutCallback(rolloutHandler);
	}
}


