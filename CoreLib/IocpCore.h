#pragma once

/*-----------------
	IOCP Core
-----------------*/

class IocpObject : public enable_shared_from_this<IocpObject>
{ //weak_ptr<IocpObject> _Wptr; // 내부적으로 weak_ptr을 상속받는다
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

private:
	HANDLE _iocpHandle;
};
