#pragma once
class IocpCore
{
public:
	IocpCore();
	virtual ~IocpCore();

private:
	HANDLE _iocpHandle;
};

