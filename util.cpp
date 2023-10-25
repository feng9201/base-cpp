#include "util.h"

#ifdef Q_OS_WIN32
#include <Windows.h>
#include <minwinbase.h>
#include <rpc.h>
#include <Psapi.h>
#include <tchar.h>
#include <shlwapi.h>
#include <string.h>
#pragma comment(lib, "version.lib")
#endif

#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QProcess>
#include <QCryptographicHash>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>

//int horizontalDPI = logicalDpiX();
//double dpi = QApplication::primaryScreen()->logicalDotsPerInch();
//	pixel = dpi*point/72    dpi point为磅值   pixel为像素值
//使用QFont的setPointSize，此时字体的大小会跟随DPI改变而改变    pt
//使用QFont的setPixelSize，此时字体的大小不会随DPI的改变而发生变化  px

#define GB (1024*1024*1024)

namespace util {
	
	QStringList findFolder(const QString& path)
	{
		QDir dir(path);
		QString folder = dir.fromNativeSeparators(path);
		QStringList allFolder = QStringList("");
		dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		dir.setSorting(QDir::Name);
		allFolder = dir.entryList();
		return  allFolder;
	}

	QStringList getFiles(const QString& path, const QString& filter)
	{
		QStringList file_list;
		QDir dir(path);
		if (!dir.exists()) {
			return file_list;
		}
		dir.setNameFilters(QStringList(filter));
		file_list = dir.entryList(QDir::Files);
		return file_list;
	}

	QString getFileSuffix(const QString& file)
	{
		QFileInfo fi(file);
		return fi.suffix();
	}

	QString getFileBaseName(const QString& file)
	{
		QFileInfo fi(file);
		return fi.completeBaseName();
	}

	bool isExistFile(const QString& filepath)
	{
		QFile file(filepath);
		return file.exists();
	}

	bool existPath(const QString& filepath)
	{
		QDir dir(filepath);
		return dir.exists();
	}

	bool delFiles(const QString& filepath)
	{
		QFileInfo info(filepath);
		if (info.isFile()) {
			QFile file(filepath);
			if (file.exists()) {
				return file.remove();
			}
		}
		else {
			QDir qdir(filepath);
			qdir.removeRecursively();
		}

		return true;
	}

	// 创建文件夹
	bool createMultiDir(const QString& sDirPath)
	{
		QStringList dirNameArray = sDirPath.split('/');
		int nameSize = dirNameArray.size();
		for (int i = 1; i < nameSize + 1; i++) {
			QString iBefAllDirStr = "";
			for (int j = 0; j < i; j++) {
				iBefAllDirStr += QString(dirNameArray.at(j) + '/');
			}

			QDir diri(iBefAllDirStr);
			if (diri.exists() == false) {
				diri.mkdir(iBefAllDirStr);
			}
		}

		return true;
	}

	bool moveFile(const QString& src, const QString& dest, bool bCover)
	{
		if (src.isEmpty() || dest.isEmpty() || !QFile().exists(src)) {
			return false;
		}

		if (src == dest)
			return true;

		if (bCover) {
			QFile file(dest);
			if (file.exists()) {
				if (!file.remove())
					return  false;
			}
		}
		else {
			// 目标文件已存在，且不允许覆盖,就认为成功了
			return true;
		}

		QFileInfo fileInfo(dest);
		QString sDirPath = fileInfo.absolutePath();
		if (!QDir().exists(sDirPath)) {
			createMultiDir(sDirPath);
		}

		// 移动文件
		QFile file(src);
		return file.rename(dest);
	}

	void selectDirPath(const QString& path)
	{
#ifdef Q_OS_WIN
		QString strPath = path;
		strPath.replace("/", "\\");
		QProcess process;
		process.startDetached("explorer", QStringList() << QString("/select,") << QString("%1").arg(strPath));
#endif // Q_OS_WIN
	}
	
	QString getTimeStamp()
	{
		QDateTime time = QDateTime::currentDateTime();
		qint64 iTime = time.toMSecsSinceEpoch();
		return QString::number(iTime);
	}

	QString FormatHHMMSS(qint32 total)
	{
		int hh = total / (60 * 60);
		int mm = (total - (hh * 60 * 60)) / 60;
		int ss = (total - (hh * 60 * 60)) - mm * 60;

		QString strTime;
		if (hh > 0) {
			strTime = QString("%1时%2分%3秒")
				.arg(hh, 2, 10, QLatin1Char('0'))
				.arg(mm, 2, 10, QLatin1Char('0'))
				.arg(ss, 2, 10, QLatin1Char('0'));
		}
		else {
			strTime = QString("%1分%2秒")
				.arg(mm, 2, 10, QLatin1Char('0'))
				.arg(ss, 2, 10, QLatin1Char('0'));
		}
		return strTime;
	}

	QString FormatHHMM(qint32 total)
	{
		int hh = total / (60 * 60);
		int mm = (total - (hh * 60 * 60)) / 60;
		QString strTime = QString("%1:%2")
			.arg(hh, 2, 10, QLatin1Char('0'))
			.arg(mm, 2, 10, QLatin1Char('0'));
		return strTime;
	}

	QString FormatMMSS(qint32 total)
	{
		int mm = total / 60;
		int ss = (total - (mm * 60)) / 60;
		QString strTime = QString("%1:%2")
			.arg(mm, 2, 10, QLatin1Char('0'))
			.arg(ss, 2, 10, QLatin1Char('0'));
		return strTime;
	}

	QString createUUid() {
		QUuid id = QUuid::createUuid();
		QString strId = id.toString();

		return strId.remove("{").remove("}").remove("-"); // 一般习惯去掉左右花括号和连字符
	}

	QString GetMd5(const QString& value)
	{
		QString md5;
		QByteArray bb;
		bb = QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Md5);
		md5.append(bb.toHex());
		return md5;
	}
	
	void filterQString(QString& name) {
		//windows不允许的字符过滤
		name.replace("/", u8"／");
		name.replace("\\", u8"＼");
		name.replace(":", u8"：");
		name.replace("*", u8"﹡");
		name.replace("?", u8"？");
		name.replace("\"", u8"“");
		name.replace("<", u8"‹");
		name.replace(">", u8"›");
		name.replace("|", u8"｜");
		//转义字符过滤
		name.replace("\n", "");
		name.replace("\r", "");
		name.replace("\t", "");
		//过滤前后空格
		name = name.simplified();
	}

	QString getRandomString(int length)
	{
		qsrand(QDateTime::currentMSecsSinceEpoch());

		const char ch[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		int size = sizeof(ch);

		char* str = new char[length + 1];
		memset(str,0, length + 1);

		int num = 0;
		for (int i = 0; i < length; ++i)
		{
			num = rand() % (size - 1);
			str[i] = ch[num];
		}

		QString res(str);
		return res;
	}
	
	QString getHostIpAddress()
	{
		QString strIpAddress;
		QString localHostName = QHostInfo::localHostName();
		//获取IP地址
		QHostInfo info = QHostInfo::fromName(localHostName);
		QList<QHostAddress> host_list = info.addresses();
		foreach(QHostAddress address, host_list) {
			if (address != QHostAddress::LocalHost &&
				address.toIPv4Address() && address.protocol() == QAbstractSocket::IPv4Protocol) {
				strIpAddress = address.toString();
				break;
			}
		}

		if (strIpAddress.isEmpty()) {
			QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
			foreach(QHostAddress address, ipAddressesList)
			{
				if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
				{
					strIpAddress = address.toString();
					break;
				}
			}
		}

		// 如果没有找到，则以本地IP地址为IP
		if (strIpAddress.isEmpty()) {
			strIpAddress = QHostAddress(QHostAddress::LocalHost).toString();
		}

		return strIpAddress;
	}
}
