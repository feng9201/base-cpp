#include "HpTcpServer.h"


HpTcpServer::HpTcpServer()
	: _tcpServer(this)
{

}

HpTcpServer::~HpTcpServer()
{

}

bool HpTcpServer::startListern(const char* host, int port)
{
	if (_tcpServer->HasStarted()) {
		return true;
	}
	return _tcpServer->Start((LPCTSTR)host,port);
}

void HpTcpServer::stopServer()
{
	if (_tcpServer->HasStarted()) {
		_tcpServer->Stop();
	}
}

void HpTcpServer::disConn(ULONG connId)
{
	if (_tcpServer->HasStarted()) {
		_tcpServer->Disconnect(connId);
	}
}

EnHandleResult HpTcpServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen)
{
	TCHAR szAddress[100];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	pSender->GetListenAddress(szAddress, iAddressLen, usPort);
	return HR_OK;
}

EnHandleResult HpTcpServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient)
{
	BOOL bPass = TRUE;
	TCHAR szAddress[100];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	pSender->GetRemoteAddress(dwConnID, szAddress, iAddressLen, usPort);

	return bPass ? HR_OK : HR_ERROR;
}

EnHandleResult HpTcpServer::OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{

	return HR_OK;
}

EnHandleResult HpTcpServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	pSender->Send(dwConnID,(BYTE*)"hello",5);
	return HR_OK;
}

EnHandleResult HpTcpServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, int iLength)
{
	ITcpPullServer* pServer = ITcpPullServer::FromS(pSender);
	if (pServer) {
		char* buffer = new char[iLength];
		EnFetchResult result = pServer->Fetch(dwConnID, (BYTE*)buffer, iLength);
		if (result == FR_OK) {
			pSender->Send(dwConnID, (BYTE*)"hello", 5);
		}
	}
	
	return HR_OK;
}

EnHandleResult HpTcpServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{

	return HR_OK;
}

EnHandleResult HpTcpServer::OnShutdown(ITcpServer* pSender)
{
	return HR_OK;
}