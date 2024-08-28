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

	bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);

	bool IsSocketValid(SOCKET socket) {
		u_long mode = 0;
		int result = ioctlsocket(socket, FIONREAD, &mode);

		if (result == SOCKET_ERROR) {
			int error = WSAGetLastError();
			// ���� �ڵ尡 WSAENOTSOCK���� Ȯ��
			cout << "SOCKET VALID ERROR: " << error << endl;
			return (error != WSAENOTSOCK);
		}
		return true;
	}

private:
	HANDLE _iocpHandle;
};
