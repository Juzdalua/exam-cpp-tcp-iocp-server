#include "pch.h"
#include "SendBuffer.h"

/*----------------
	SendBuffer
-----------------*/

SendBuffer::SendBuffer(int32 bufferSize)
{
	_buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::CopyData(void* data, int32 len)
{
	ASSERT_CRASH(Capacity() >= len);
	::memcpy(_buffer.data(), data, len);
	_writeSize = len;
}

void SendBuffer::CopyPacket(BYTE* pos,void* data, int32 len)
{
	ASSERT_CRASH(Capacity() >= len);
	::memcpy(pos, data, len);
	_writeSize = len + sizeof(PacketHeader);
}
