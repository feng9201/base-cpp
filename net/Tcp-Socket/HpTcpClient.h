#pragma once
#include "HPSocket/HPSocket.h"
#include <memory>
#include <functional>

enum class HpStatus {
	Idel = 0,
	Connected,
	Close
};

using HpTcpStatusCB = std::function<void(HpStatus status,const char* msg)>;
class HpTcpClient :public CTcpPullClientListener
{
public:
	HpTcpClient();
	~HpTcpClient();

	bool	Connect(const char* ip,int port, HpTcpStatusCB statusCb ,bool asyncConn = true);
	bool	SendData(const char* data,int length);

	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);

private:
	bool	_IsConnect = false;

	CTcpPullClientPtr	_tcp_client;
	HpTcpStatusCB		_status_cb;
};

