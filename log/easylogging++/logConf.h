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
		/// ������־
		QFile file(filename);
		QFileInfo fileInfo(file); // ��ȡԭʼ�ļ���Ϣ
		QString newFilePath = fileInfo.path() + "/" + fileInfo.baseName() + "_" + QString::number(idx) + ".log"; // ƴ���µ��ļ�·��
		if (idx > 2) {
			idx = 0;
		}
		QFile newfile(newFilePath);
		if (newfile.exists()) {
			newfile.remove();
		}
		if (!file.rename(newFilePath)) { // �������ļ�
			int i = 0;
		}
		else {
			idx++;
		}
	}

	void	installLog()
	{
		el::Configurations conf;
		// ������־
		conf.setGlobally(el::ConfigurationType::Enabled, "true");
		//������־�ļ�Ŀ¼�Լ��ļ���
		QString path = QCoreApplication::applicationDirPath();
		path = path.append("\\log\\log_%datetime{%Y%M%d_%H%m%s}.log");
		conf.setGlobally(el::ConfigurationType::Filename, path.toStdString());
		//������־�ļ�����ļ���С 10M
		conf.setGlobally(el::ConfigurationType::MaxLogFileSize, "10485760");
		//�Ƿ�д���ļ�
		conf.setGlobally(el::ConfigurationType::ToFile, "true");
		//�Ƿ��������̨
		conf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
		//������־�����ʽ
		conf.setGlobally(el::ConfigurationType::Format, "[%datetime] [%loc] [%level] : %msg");
		//������־�ļ�д�����ڣ�����ÿ100��ˢ�µ��������
		conf.setGlobally(el::ConfigurationType::LogFlushThreshold, "10");
		//���������ļ�
		el::Loggers::reconfigureAllLoggers(conf);


		el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
		/// ��ֹFatal������־�жϳ���
		el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
		/// ѡ�񻮷ּ������־	
		el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
		/// ���ü����ŷ�ֵ���޸Ĳ������Կ�����־���
		el::Loggers::setLoggingLevel(el::Level::Fatal);
		// ��־���洢�ص�
		el::Helpers::installPreRollOutCallback(rolloutHandler);
	}
}


