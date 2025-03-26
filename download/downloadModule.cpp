#include "stdafx.h"
#include "downloadModule.h"
#include "httpUrlSplit.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <future>
#include <thread>
#include <sys/stat.h>


namespace download {

    URLDownloadTask::URLDownloadTask(const strc_downTask& downInfo)
        : _downInfo(downInfo)
    {
        _http_client.reset();
    }

    URLDownloadTask::~URLDownloadTask()
    {
    }

    int64_t URLDownloadTask::fileSize(const std::string& filePath)
    {
        struct stat statbuf;
        stat(filePath.c_str(), &statbuf);
        size_t filesize = statbuf.st_size;
        return filesize;
    }

    void URLDownloadTask::startDownload(DownloadStatusNotifyCB downloadCB)
    {
        _status_cb = downloadCB;
        auto task = [=] {
            std::string url = _downInfo.url;
            std::string local_path = _downInfo.localpath + "/" + _downInfo.fileName;
            if (!_downInfo.fileSuffix.empty()) {
                local_path = local_path + "." + _downInfo.fileSuffix;
            }
            _download_path = local_path;
            //std::cout<<"start download------------------------------->"<<_download_path;
            // 临时文件
            std::string tmp_part = local_path + ".part";
            std::string resume_file = _downInfo.localpath + "/" + _downInfo.fileName + ".ini";

            //移除旧文件
            std::remove(_download_path.c_str());

            int64_t resume_size = 0;
            if (!_downInfo.support_resume) {
                std::remove(tmp_part.c_str());
            }
            else {//支持断点续传
                std::ifstream f(resume_file);
                if (f.good()) {
                    std::ifstream ifs;
                    ifs.open(resume_file, std::ios::in | std::ios::binary);
                    if (ifs.is_open()) {
                        std::string s;
                        std::string tmpFile;
                        while (getline(ifs, s)) {
                            tmpFile.append(s);
                        }
                        ifs.close();
                        if (tmpFile == url) {
                            resume_size = fileSize(tmp_part);
                        }
                    }
                }
            }

            // 断点续传记录下载地址
            if (resume_size == 0 && _downInfo.support_resume) {
                std::ofstream ofs;
                ofs.open(resume_file, std::ios::out | std::ios::trunc);
                if (ofs.is_open()) {
                    ofs << _downInfo.url;
                }
                ofs.close();
            }

            unsigned short port;
            char host[256];
            memset(host, 0, sizeof(host));

            char url_path[512];
            memset(url_path, 0, sizeof(url_path));
            bool ret = httpUrlSplit::get_addr(url.c_str(), host, url_path, sizeof(url_path), &port);
            if (!ret) {
                LOG_CPP(w_log_cpp::LOG_LEVEL_E::WARN," getAddr fail url:%s~",url_path);
                downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, 0, _downInfo, _download_path);
                return;
            }

            std::ofstream file;
            if (resume_size != 0) {
                file.open(tmp_part.c_str(), std::ios::out | std::ios::binary | std::ios::app);
            }
            else {
                file.open(tmp_part.c_str(), std::ios::out | std::ios::binary);
            }

            if (!file.is_open()) {
                LOG_CPP(w_log_cpp::LOG_LEVEL_E::WARN," create tmp part fail :%s~",tmp_part.c_str());
                downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, 0, _downInfo, _download_path);
                return;
            }

            if (_stop_download) {
                return;
            }

            if (port == 0) {
                size_t index = url.find(url_path);
                if (index > url.size()) {
                    LOG_CPP(w_log_cpp::LOG_LEVEL_E::WARN," find host port fail :%s~",url.c_str());
                    downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, 0, _downInfo, _download_path);
                    return;
                }
                std::string domain = url.substr(0, index);
                _http_client.reset(new httplib::Client(domain));
            }
            else {
                _http_client.reset(new httplib::Client(host, port));
            }
            _http_client->set_connection_timeout(0, 300000);
            _http_client->set_read_timeout(3, 0);
            _http_client->set_follow_location(true);
            _http_client->set_keep_alive(true);
            _http_client->enable_server_certificate_verification(false);

            int64_t total_size = 0;
            int64_t download_size = 0;
            auto res = _http_client->Get(url_path,
                { httplib::make_range_header({ {resume_size, -1} }) },	//断点续传
                [&](const httplib::Response& response) {
                    auto iter = response.headers.find("Content-Length");
                    if (iter != response.headers.end()) {
                        if (resume_size == 0) {
                        }
                    }
                    return true;
                },
                [&](const char* data, size_t data_length) {
                    if (_stop_download) {
                        return false;
                    }
                    download_size += data_length;
                    //存储下载数据
                    file.write(data, data_length);
                    return true;
                },
                [=, &total_size](uint64_t len, uint64_t total) {
                    if (_stop_download) {
                        return false;
                    }
                    total_size = total;
                    downloadCB(downloadStatus::DOWNLOAD_PROGRESS, len, total_size + resume_size, _downInfo, _download_path);
                    return true;
                });
            if (res.error() != httplib::Error::Success) {
                if (!_downInfo.support_resume) {
                    std::remove(tmp_part.c_str());
                }

                LOG_CPP(w_log_cpp::LOG_LEVEL_E::WARN," url download fail err:%d ; url :%s ", (int)res.error(),url.c_str());
                _http_client.reset();
                file.close();
                _downInfo.download_count = _downInfo.download_count + 1;
                downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, total_size, _downInfo, _download_path);
                return;
            }
            _http_client.reset();
            file.close();

            //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            if (download_size == total_size && total_size != 0) {
                if (_downInfo.support_resume) {
                    std::remove(resume_file.c_str());
                }
                auto file_len = fileSize(tmp_part);
				if (file_len < 1024*8) {//
					LOG_CPP(w_log_cpp::LOG_LEVEL_E::ERR, "[download] fail part_size <8kb total_size:%lld ; id:%s", total_size,_downInfo.task_id.c_str());
					if (!_downInfo.support_resume) {
						std::remove(tmp_part.c_str());
					}
					_downInfo.download_count = _downInfo.download_count + 1;
					downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, total_size + resume_size, _downInfo, _download_path);
					return;
				}

                int ret = std::rename(tmp_part.c_str(), _download_path.c_str());
                if (ret == 0) {
                    LOG_CPP(w_log_cpp::LOG_LEVEL_E::INFO, "[download] download ok size:%lld", total_size);
                    downloadCB(downloadStatus::DOWNLOAD_SUCCESS, download_size + resume_size, total_size + resume_size, _downInfo, _download_path);
                }
                else {
                    LOG_CPP(w_log_cpp::LOG_LEVEL_E::ERR, "[download] rame fail id:%s", _downInfo.task_id.c_str());
                    _downInfo.download_count = _downInfo.download_count + 1;
                    downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, total_size + resume_size, _downInfo, _download_path);
                    std::remove(tmp_part.c_str());
                }
            }
            else {
                if (!_downInfo.support_resume) {
                    std::remove(tmp_part.c_str());
                }
                _downInfo.download_count = _downInfo.download_count + 1;
                downloadCB(downloadStatus::DOWNLOAD_FAIL, 0, total_size + resume_size, _downInfo, _download_path);
            }
            };

        std::thread thread(task);
        thread.detach();
    }

    void URLDownloadTask::stopTask()
    {
        _stop_download = true;
    }

    downloadModule::downloadModule()
    {}

    downloadModule::~downloadModule()
    {}

    bool downloadModule::addTask(const strc_downTask& downInfo)
    {
        if (downInfo.url.empty() || downInfo.localpath.empty()) {
            if (_events.download_status) {
                _events.download_status(downloadStatus::DOWNLOAD_FAIL, downInfo, "");
            }
            return false;
        }

        if (checkHasTask(downInfo.url))
        {
            if (_events.download_status) {
                _events.download_status(downloadStatus::DOWNLOAD_REPEAT, downInfo, "");
            }
            return false;
        }

        if (canDownload())
        {
            recordUrlMap(downInfo.url, downInfo);

            std::string file_path = getFilePath(downInfo);

            if (downInfo.use_cache && fileExist(file_path))
            {
                struct stat statbuf;
                stat(file_path.c_str(), &statbuf);
                size_t filesize = statbuf.st_size;
                if (filesize < 10) {
                    std::remove(file_path.c_str());
                    executeTask(downInfo);
                    return true;
                }
                if (_events.download_status) {
                    _events.download_status(downloadStatus::DOWNLOAD_SUCCESS, downInfo, file_path);
                }
                return true;
            }
            else
            {
                executeTask(downInfo);
            }
        }
        else
        {
            if (_events.download_status) {
                _events.download_status(downloadStatus::DOWNLOAD_WAIT, downInfo, "");
            }
            addWaitingTask(downInfo.url, downInfo);
        }
        return true;
    }

    void downloadModule::executeTask(const strc_downTask& downInfo)
    {
        if (_events.download_status) {
            _events.download_status(downloadStatus::DOWNLOAD_START, downInfo, "");
        }

        std::unique_ptr<URLDownloadTask> task_ptr = std::make_unique<URLDownloadTask>(downInfo);
        {
            std::lock_guard<std::mutex> guard(current_task_mutex_);
            _current_tasks[downInfo.url] = std::move(task_ptr);
            _current_tasks[downInfo.url]->startDownload([=](downloadStatus status, int64_t download_byte, int64_t total_byte, const strc_downTask& info, const std::string& down_path) {
                notifyDownloadResult(status, download_byte, total_byte, info, down_path);
                });
        }
    }

    void downloadModule::notifyDownloadResult(downloadStatus status, int64_t download_byte, int64_t total_byte, const strc_downTask& down_task, const std::string& down_path)
    {
        if (status == downloadStatus::DOWNLOAD_PROGRESS) {
            if (_events.download_progress) {
                int progress = (int)(((float)download_byte / total_byte) * 100);
                _events.download_progress(down_task, progress, download_byte, total_byte);
            }
        }
        else {
            if (_events.download_status) {
                _events.download_status(status, down_task, down_path);
            }
        }

        if (status == downloadStatus::DOWNLOAD_FAIL || status == downloadStatus::DOWNLOAD_TIMEOUT || status == downloadStatus::DOWNLOAD_SUCCESS) {
            checkWaitingQueue();
            removeTask(down_task.url);
        }
    }

    void downloadModule::addWaitingTask(const std::string& url, const strc_downTask& info)
    {
        _waiting_download_task[url].push_back(info);
    }

    void downloadModule::checkWaitingQueue()
    {
        auto task = [this]
            {
                int can_consume_size = _max_task_num - _current_tasks.size() + 1;
                if (can_consume_size > 0 && !_waiting_download_task.empty())
                {
                    can_consume_size = 2;
                    int waiting_size = _waiting_download_task.size();
                    auto it = _waiting_download_task.begin();
                    for (int i = 0, j = 0; i < can_consume_size && j < waiting_size; ++i, ++j, ++it)
                    {
                        executeTask(it->second.at(0));
                        recordUrlMap(it->first, std::move(it->second));
                    }

                    _waiting_download_task.erase(_waiting_download_task.begin(), it);
                }
            };

        std::async(std::launch::async, task);
    }

    std::string downloadModule::getFilePath(const strc_downTask& download_task)
    {
        std::string path = download_task.localpath + "/" + download_task.fileName;
        if (!download_task.fileSuffix.empty()) {
            path = path + "." + download_task.fileSuffix;
        }
        return path;
    }

    void downloadModule::removeTask(const std::string& url)
    {
        std::lock_guard<std::mutex> guard(current_task_mutex_);
        _current_tasks.erase(url);
    }

    void downloadModule::recordUrlMap(const std::string& url, const strc_downTask& game_info)
    {
        _url_map_ids[url].push_back(game_info);
    }

    void downloadModule::recordUrlMap(const std::string& url, std::vector<strc_downTask> ids)
    {
        _url_map_ids[url] = std::move(ids);
    }

    bool downloadModule::canDownload()
    {
        std::lock_guard<std::mutex> guard(current_task_mutex_);
        return _current_tasks.size() < _max_task_num;
    }

    bool downloadModule::checkHasTask(const std::string& url)
    {
        std::lock_guard<std::mutex> guard(current_task_mutex_);
        return _current_tasks.find(url) != _current_tasks.cend();
    }

    bool downloadModule::fileExist(const std::string& file_path)
    {
        std::ifstream f(file_path.c_str());
        return f.good();
    }

    void downloadModule::stopAll()
    {
        if (_current_tasks.size() > 0) {
            _current_tasks.clear();
        }
        if (_waiting_download_task.size() > 0) {
            _waiting_download_task.clear();
        }
    }

    void downloadModule::stopTask(const std::string& url)
    {
        std::lock_guard<std::mutex> guard(current_task_mutex_);
        _current_tasks.erase(url);
        if (_current_tasks.find(url) != _current_tasks.cend()) {
            _current_tasks[url]->stopTask();
        }
    }

    void downloadModule::setMaxTasks(int num)
    {
        _max_task_num = num;
    }
}

