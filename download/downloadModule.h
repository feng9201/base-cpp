#pragma once
/*
@author：hxf
@time：2023-10-17
@function：多线程下载类封装，依赖第三方库httplib
@使用例子
    download::downloadModule* down_module = new download::downloadModule();
    down_module->events().download_status = [=](download::downloadStatus status, const download::strc_downTask& task, const std::string& download_path) {
        //业务状态逻辑处理
    };
    down_module->events().download_progress = [=](const download::strc_downTask& task, int progress, uint64_t download_byte, uint64_t total_byte) {
        //下载进度逻辑处理
    };
    download::strc_downTask down_task;
    down_module->addTask(std::move(down_task));
*/

#include <string>
#include <memory>

namespace httplib {
    class Client;
}

namespace download {
    struct strc_downTask {
        std::string		url;						//下载地址
        std::string		localpath;					//文件路径，不包括文件名
        std::string		fileName;					//文件保存名称
        std::string		fileSuffix;					//后缀名，为空时认为fileName带有后缀
        bool			support_resume = false;		//是否支持断点续传
        bool			use_cache = false;			//之前已经下载过了，直接使用之前下载的文件
        std::string		task_id;					//任务唯一ID,业务方可用于判断下载的任务

        //本需求需要增添
        bool            insert_flag = false;
        int				download_count = 0;			//下载次数
    };

    enum class downloadStatus {
        DOWNLOAD_WAIT = 0,
        DOWNLOAD_START,
        DOWNLOAD_REQ,
        DOWNLOAD_PROGRESS,
        DOWNLOAD_FAIL,
        DOWNLOAD_REPEAT,
        DOWNLOAD_TIMEOUT,
        DOWNLOAD_SUCCESS
    };

    using DownloadStatusNotifyCB = std::function<void(downloadStatus status, int64_t download_byte, int64_t total_byte, const strc_downTask& info, const std::string& down_path)>;

    class URLDownloadTask {
    public:
        URLDownloadTask(const strc_downTask& downInfo);
        ~URLDownloadTask();

        void	startDownload(DownloadStatusNotifyCB downloadCB);
        void	stopTask();

    private:
        int64_t fileSize(const std::string& filePath);

    private:
        bool	_stop_download = false;
        strc_downTask							_downInfo;
        std::string								_download_path;
        std::unique_ptr<httplib::Client>		_http_client;
        DownloadStatusNotifyCB					_status_cb;
    };

    struct Events {

        using DownloadStatusHandler = std::function<void(downloadStatus, const strc_downTask& task, const std::string& download_path)>;
        using DownloadingProgressHandler
            = std::function<void(const strc_downTask& task, int progress, uint64_t download_byte, uint64_t total_byte)>;
        DownloadStatusHandler		download_status;
        DownloadingProgressHandler  download_progress;
    };

    class downloadModule
    {
    public:
        downloadModule();
        ~downloadModule();

        Events& events() { return _events; }

        bool	addTask(const strc_downTask& downInfo);
        void	stopTask(const std::string& url);
        void	stopAll();
        void	setMaxTasks(int num);
        bool    isEmpty() { return _current_tasks.empty(); };
    private:
        void   notifyDownloadResult(downloadStatus status, int64_t download_byte, int64_t total_byte, const strc_downTask& info, const std::string& down_path);

        void	executeTask(const strc_downTask& downInfo);
        void	removeTask(const std::string& url);
        void	checkWaitingQueue();
        void    recordUrlMap(const std::string& url, const strc_downTask& game_info);
        void    recordUrlMap(const std::string& url, std::vector<strc_downTask> ids);
        bool	checkHasTask(const std::string& url);
        void	addWaitingTask(const std::string& url, const strc_downTask& info);
        bool	canDownload();
        std::string	getFilePath(const strc_downTask& download_task);

        bool	fileExist(const std::string& file_path);
    private:
        std::mutex		current_task_mutex_;
        std::map<std::string, std::vector<strc_downTask>>		_url_map_ids;					//执行中
        std::map<std::string, std::vector<strc_downTask>>		_waiting_download_task;			//等待中
        std::map<std::string, std::unique_ptr<URLDownloadTask>>	_current_tasks;

        Events	_events;
        int		_max_task_num = 5;
    };

}


