#include "HpTcpClient.h"


HpTcpClient::HpTcpClient()
	: _tcp_client(this)
{

}

HpTcpClient::~HpTcpClient()
{

}

bool HpTcpClient::Connect(const char* ip, int port, HpTcpStatusCB statusCb, bool asyncConn)
{
	if (_IsConnect) {
		return true;
	}
	_status_cb = statusCb;

	if (_tcp_client->Start((LPCTSTR)ip, port, asyncConn)) {
		return true;
	}
	return false;
}

bool HpTcpClient::SendData(const char* data, int len)
{
	if (!_IsConnect || len<=0 || !data) {
		return false;
	}
	return _tcp_client->Send((BYTE*)data,len);
}

EnHandleResult HpTcpClient::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
	TCHAR szAddress[100];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;
	pSender->GetLocalAddress(szAddress, iAddressLen, usPort);

	if (_status_cb) {
		_status_cb(HpStatus::Connected,"connect success");
	}
	_IsConnect = true;
	return HR_OK;
}

EnHandleResult HpTcpClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	return HR_OK;
}

EnHandleResult HpTcpClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, int iLength)
{
	ITcpPullClient* pClient = ITcpPullClient::FromS(pSender);

	char* buffer = new char[iLength];
	EnFetchResult result = pClient->Fetch((BYTE*)buffer, iLength);
	if (result == FR_OK) {
		int i = 0;
	}
	return HR_OK;
}

EnHandleResult HpTcpClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	return HR_OK;
}

EnHandleResult HpTcpClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	_IsConnect = false;
	if (_status_cb) {
		_status_cb(HpStatus::Close, "connect close");
	}
	return HR_OK;
}