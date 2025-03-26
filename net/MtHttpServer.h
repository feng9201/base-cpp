#pragma once
#include <atomic>
#include <memory>
#include <thread>
#include <string>
#include "mongoose.h"


namespace mttool
{
	class MtHttpServer
	{
	public:
		MtHttpServer();
		~MtHttpServer();

		bool startHttpServer(const char* host,std::pair<std::string,std::string>,int timeout = 5000);
		void stopServer();
	private:
		friend void mtHttpFunc(struct mg_connection* c, int ev, void* ev_data);

		void testFunc();
	private:
		std::atomic<bool>  _init_ws{ false };
		std::atomic<bool>  _is_stop{ false };
		std::unique_ptr<std::thread> _worker_thread;
		std::thread::id _worker_td_id;
		struct mg_mgr _mgr;  
		struct mg_connection* _ctmp = nullptr;

		std::string _req_methed;
		std::string _req_uri;
	};
}


