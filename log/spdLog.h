#pragma once
#include <QString>

namespace util {
	
	/// @brief Qt调试输出拦截
	/// @param type 
	/// @param context 
	/// @param msg 
	void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg);

	/// @brief 初始化spdlog日志系统
	void initSpdlog(QString logPath);
}


