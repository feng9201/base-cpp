#include "MtWsClient.h"
#include <thread>
#include <string>

#ifdef WS_USE_OPENSSL
#define MG_TLS MG_TLS_OPENSSL
#endif
namespace mttool {

	MtWsClient::MtWsClient()
	{
	}

	MtWsClient::~MtWsClient()
	{
		stopWs();
	}

	void wsFn(struct mg_connection* c, int ev, void* ev_data) 
	{
		MtWsClient* ws = nullptr;
		if (c && c->fn_data) {
			ws = (MtWsClient*)(c->fn_data);
		}
		if (ev == MG_EV_ERROR) {
			const char* err_msg = (const char*)ev_data;
			if (ws) {
#ifdef	WS_USE_OVERRIDE
				ws->onError(err_msg);
#endif
				if (ws->_events.onError) {
					ws->_events.onError(err_msg);
				}
				//ws->stopWs();
			}
		}
		else if (ev == MG_EV_ACCEPT) {
			//若是wss则跳过证书，没有证书
			struct mg_tls_opts opts;
			opts.skip_verification = 1;
			mg_tls_init(c, &opts);
		}
		else if (ev == MG_EV_WS_OPEN) {//MG_EV_CONNECT现有连接，在有open
			if (ws) {
#ifdef	WS_USE_OVERRIDE
				ws->onConn();
#endif
				if (ws->_events.onConn) {
					ws->_events.onConn();
				}
			}
		}
		else if (ev == MG_EV_WS_MSG) {
			struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
			if (wm && ws) {
#ifdef	WS_USE_OVERRIDE
				ws->onReceiveMsg(wm->data.buf, (int)wm->data.len);
#endif
				if (ws->_events.onReceiveMsg) {
					ws->_events.onReceiveMsg(wm->data.buf, (int)wm->data.len);
				}
			}
		}
		else if (ev == MG_EV_CLOSE) {
			if (ws) {
#ifdef	WS_USE_OVERRIDE
				ws->onDisconn();
#endif
				if (ws->_events.onDisconn) {
					ws->_events.onDisconn();
				}
				ws->stopWs();
			}
		}
	}

	bool MtWsClient::sendMsg(const char* msg, int len)
	{
		if (msg && _mg_conn) {
			auto size = mg_ws_send(_mg_conn, msg, len, WEBSOCKET_OP_TEXT);
			if (size == len) {
				return true;
			}
		}
		return false;
	}

	void MtWsClient::stopWs()
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
				freeWs();
			}
		}
	}

	bool MtWsClient::isConn()
	{
		if (_init_ws.load() && _mg_conn) {
			return true;
		}
		return false;
	}

	void MtWsClient::freeWs()
	{
		if (_init_ws.load()) {
			_init_ws.store(false);
			mg_mgr_free(&_mgr);
			_mg_conn = nullptr;
		}
	}

	bool MtWsClient::openWs(const char* url, int timeout)
	{
		if (_init_ws.load()) {
			return false;
		}
		if (_worker_thread) {
			_worker_thread->join();
			_worker_thread.reset();
		}
		mg_mgr_init(&_mgr);
		_mgr.dnstimeout = timeout;
		_is_stop.store(false);
		_mg_conn = mg_ws_connect(&_mgr, url, &wsFn,this, nullptr);
		if (!_mg_conn || _is_stop) {
			mg_mgr_free(&_mgr);
			return false;
		}
		_init_ws.store(true);
		_worker_thread = std::make_unique<std::thread>([this] {
			_worker_td_id = std::this_thread::get_id();
			while (!_is_stop.load() && _mg_conn) {
				mg_mgr_poll(&_mgr, 100);
			}
			freeWs();
		});
		return true;
	}
};