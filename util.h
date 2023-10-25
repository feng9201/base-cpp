#pragma once

#include <QObject>
#include <QString>

namespace util{

	/*******文件操作**************/
	// 获取路径下所有文件夹名称
	QStringList findFolder(const QString& path);

	// 获取路径下所有文件
	QStringList getFiles(const QString& path, const QString& filter = "*.*");

	// 取文件后缀名(*.png png)
	QString getFileSuffix(const QString& file);

	// 获取文件名(文件名.*)
	QString getFileBaseName(const QString& file);

	// 判断文件是否存在
	bool isExistFile(const QString& filepath);

	// 判断文件夹路径是否存在
	bool existPath(const QString& filepath);

	// 删除文件--若是文件夹则全部删除
	bool delFiles(const QString& filepath);

	// 创建多级目录文件夹
	bool createMultiDir(const QString& sDirPath);

	// 移动文件
	bool moveFile(const QString& src, const QString& des, bool bCover = true);

	// 打开文件夹，并选中
	void selectDirPath(const QString& path);
	
	/*******文件操作 end**************/

	// 获取时间戳到秒
	QString getTimeStamp();

	// 秒转时分秒
	QString FormatHHMMSS(qint32 total);

	// 秒转时分
	QString FormatHHMM(qint32 total);

	// 秒转分秒
	QString FormatMMSS(qint32 total);

	//md5加密
	QString GetMd5(const QString& value);
	
	// 过滤字符
	void filterQString(QString& name);

	// 随机字符串
	QString getRandomString(int length);
	
	// uuid
	QString createUUid();
	
	QString getHostIpAddress();
}
