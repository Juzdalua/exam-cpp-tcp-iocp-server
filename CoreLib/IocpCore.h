#pragma once

class IocpObject : public enable_shared_from_this<IocpObject>
{
	//weak_ptr<IocpObject> _Wptr; // ���������� weak_ptr�� ��ӹ޴´�
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

class IocpCore
{
public:
	IocpCore();
	virtual ~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(HANDLE iocpHandle);
	void Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle;
};

