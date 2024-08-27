#pragma once

/*-----------------
	IOCP Core
-----------------*/

class IocpObject : public enable_shared_from_this<IocpObject>
{ //weak_ptr<IocpObject> _Wptr; // ���������� weak_ptr�� ��ӹ޴´�
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/*-----------------
	IOCP Core
-----------------*/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	/*bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);*/

	// TEMP
	bool Register(SOCKET& socket);
	bool Register(shared_ptr<Session> session);
	bool Dispatch(SOCKET& listenSocket);

	void RegisterAccept(SOCKET& listenSocket);
	void ProcessWorker(IocpEvent* iocpEvent, DWORD numOfBytes, SOCKET& listenSocket);
	vector<IocpEvent*> _iocpEvents;
	
private:
	HANDLE _iocpHandle;
};
