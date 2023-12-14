#pragma once
#include "HPSocket/HPSocket.h"


class HpTcpServer :
    public CTcpPullServerListener
{
public:
	HpTcpServer();
	~HpTcpServer();

	bool	startListern(const char* host,int port);
	void	disConn(ULONG connId);
	void	stopServer();
protected:
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, int iLength);

private:
	CTcpPullServerPtr		_tcpServer;

};

