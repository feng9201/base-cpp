#pragma once
#include <QDir>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QProcess>
#include <qDebug>

#ifdef Q_OS_WIN32
#include <Windows.h>
#include <DbgHelp.h>
#include <errhandlingapi.h>

#pragma comment(lib, "Dbghelp.lib")
#endif


namespace {
	//异常捕获函数
#ifdef Q_OS_WIN32
	static long ApplicationCrashHandler(EXCEPTION_POINTERS* pException)
	{
        QString TempPath = qApp->applicationDirPath();
        QString DumpDirPath = TempPath + "/Dumps/";
        QDir dir(DumpDirPath);
        if (!dir.exists()) {
            dir.mkdir(DumpDirPath);
        }

        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyyMMdd_hhmmss");
        QString fileName = QString("%1_%2.dmp").arg(current_date).arg("v1.0.1");

        HANDLE hDumpFile = CreateFile((LPCSTR)QString(DumpDirPath + fileName).utf16(),
            GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hDumpFile != INVALID_HANDLE_VALUE)
        {
            HMODULE hModule = NULL;
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pException->ExceptionRecord->ExceptionAddress,
                &hModule);
            wchar_t szModuleName[MAX_PATH] = { 0 };

            if (hModule) {

                GetModuleFileName(hModule, (LPSTR)szModuleName, ARRAYSIZE(szModuleName));
            }

            MINIDUMP_EXCEPTION_INFORMATION ExpParam;

            ExpParam.ThreadId = GetCurrentThreadId();
            ExpParam.ExceptionPointers = pException;
            ExpParam.ClientPointers = TRUE;

            MINIDUMP_TYPE MiniDumpWithDataSegs = (MINIDUMP_TYPE)(MiniDumpNormal
                | MiniDumpWithHandleData
                | MiniDumpWithUnloadedModules
                | MiniDumpWithIndirectlyReferencedMemory
                | MiniDumpScanMemory
                | MiniDumpWithProcessThreadData
                | MiniDumpWithThreadInfo);

            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

            /*QString dmpPath = DumpDirPath + fileName;
            QFile fileDmp("./dmpFileInfo.txt");

            if (fileDmp.open(QIODevice::WriteOnly))
            {
                fileDmp.write(dmpPath.toLatin1());
                fileDmp.flush();
                fileDmp.close();
            }
            {
                QFile fileCrash("./Crash.txt");
                if (fileCrash.open(QIODevice::Append))
                {
                    fileCrash.write(QString::fromStdWString(szModuleName).toLocal8Bit().data());
                    fileCrash.flush();
                    fileCrash.close();
                }
            }*/
        }
        else {
            qDebug() << "hDumpFile == null";
        }
        return EXCEPTION_EXECUTE_HANDLER;
	}
#endif

}
