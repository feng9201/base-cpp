#pragma once
/*****
* @brief:基于mongoose的websocket客户端类
* @auth:hxf
* @date:2025-3-25
* @WS_USE_OPENSSL  该宏代表使用openssl
*   使用方式：1.基于回调的使用方式
*   MtWsClient wsClient;
	wsClient.events().onConn = [=] {
		std::cout << "conn ok" << std::endl;
	};
	wsClient.events().onError = [=](const char* err_msg) {
		std::cout << "conn error:" << err_msg<< std::endl;
	};
	wsClient.events().onDisconn = [=] {
		std::cout << "disconn:" << std::endl;
	};
	wsClient.events().onReceiveMsg = [=](const char* msg,int len) {
		std::cout << "onReceiveMsg:" << msg << std::endl;
	};
	2.基于继承的使用方式：打开宏 WS_USE_OVERRIDE
******/
#include "mongoose.h"
#include <functional>
#include <atomic>
#include <thread>
#include <memory>


namespace mttool {
	class MtWsClient
	{
	public:
		MtWsClient();
		virtual ~MtWsClient();

		struct wsEvents {

			using wsConnHandler = std::function<void()>;
			using wsErrorHandler = std::function<void(const char* err_msg)>;
			using wsDisconnHandler = std::function<void()>;
			using wsReceiveMsgHandler = std::function<void(const char* err_msg, int len)>;
			
			wsConnHandler		onConn;
			wsErrorHandler		onError;
			wsDisconnHandler    onDisconn;
			wsReceiveMsgHandler onReceiveMsg;
		};

		bool openWs(const char* url, int timeout = 5000);
		bool sendMsg(const char* msg,int len);
		//不要从回调或者继承函数调用stop，可能存在进程退出，线程未退出问题
		void stopWs();
		bool isConn();
		wsEvents& events() { return _events; }
	protected:
#ifdef  WS_USE_OVERRIDE
		virtual void onConn() = 0;
		virtual void onError(const char* err_msg) = 0;
		virtual void onDisconn() = 0;
		virtual void onReceiveMsg(const char* msg,int len)=0;
#endif
	private:
		friend void wsFn(struct mg_connection* c, int ev, void* ev_data);
		void freeWs();
	private:
		std::atomic<bool>   _init_ws{false};
		std::atomic<bool>  _is_stop{false};
		//std::thread _worker_thread;
		std::unique_ptr<std::thread> _worker_thread;
		std::thread::id _worker_td_id;
		wsEvents _events;
		mg_mgr _mgr;
		mg_connection* _mg_conn = nullptr; 
		int _timeout = 5000;//超时时间ms
	};
};
