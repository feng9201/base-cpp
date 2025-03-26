#include "MtHttpServer.h"
#include <functional>


namespace mttool
{
	MtHttpServer::MtHttpServer()
	{
	}
	MtHttpServer::~MtHttpServer()
	{
	}

	void mtHttpFunc(struct mg_connection* c, int ev, void* ev_data) 
	{
		MtHttpServer* mt_server = nullptr;
		if (c && c->fn_data) {
			mt_server = (MtHttpServer*)(c->fn_data);
		}
		if (!mt_server) {
			return;
		}
		if (ev == MG_EV_HTTP_MSG) {  
			struct mg_http_message* hm = (struct mg_http_message*)ev_data;  
			if (mg_match(hm->method, mg_str(mt_server->_req_methed.c_str()), NULL)) {
				if (mg_match(hm->uri, mg_str(mt_server->_req_uri.c_str()), NULL)) {
					std::string bodys = std::string(hm->body.buf, hm->body.len);
					//支持异步回调信息
					mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), 23); 
				}
				else {
					struct mg_http_serve_opts opts;
					opts.root_dir = ".";
					mg_http_serve_dir(c, hm, &opts);                     
				}
			}
		}
	}

	void MtHttpServer::testFunc()
	{
		while (true) {
			if (_ctmp) {
				mg_http_reply(_ctmp, 200, "", "{%m:%d}\n", MG_ESC("status"), 11);
				_ctmp = nullptr;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(15000));
		}
	}

	void MtHttpServer::stopServer()
	{
		if (!_is_stop.load()) {
			_is_stop.store(true);
			auto this_id = std::this_thread::get_id();
			if (_worker_thread && _worker_thread->joinable()) {
				if (_worker_td_id != this_id) {
					_worker_thread->join();
					_worker_thread.reset();
				}
			}
			if (_worker_td_id != this_id) {
				_init_ws.store(false);
				mg_mgr_free(&_mgr);
			}
		}
	}

	bool MtHttpServer::startHttpServer(const char* host, std::pair<std::string, std::string> pairs,int timeout)
	{
		if (_init_ws.load()) {
			return false;
		}

		_req_methed = pairs.first;
		_req_uri = pairs.second;

		mg_mgr_init(&_mgr);
		_mgr.dnstimeout = timeout;
		_is_stop.store(false);
		auto mg_conn = mg_http_listen(&_mgr, host, mtHttpFunc, this);
		if (!mg_conn) {
			return false;
		}
		_init_ws.store(true);
		_worker_thread = std::make_unique<std::thread>([this] {
			_worker_td_id = std::this_thread::get_id();
			while (!_is_stop.load()) {
				mg_mgr_poll(&_mgr, 800);
			}
		});
		return true;
	}
}