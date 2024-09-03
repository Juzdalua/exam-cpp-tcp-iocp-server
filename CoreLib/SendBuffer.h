#pragma once

class SendBuffer : enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int32 bufferSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer.data(); }
	int32 WriteSize() { return _writeSize; }
	int32 Capacity() { return static_cast<int32>(_buffer.size()); }

	void CopyData(void* data, int32 len);
	void CopyPacket(BYTE* pos, void* data, int32 len);

	void SetWriteSizeWithDataSize(int32 len){ _writeSize = len + sizeof(PacketHeader); }

private:
	vector<BYTE>	_buffer;
	int32			_writeSize = 0;
};

template<typename T>
SendBufferRef MakeSendBuffer(T& pkt, uint16 packetId) {
	const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
	const uint16 packetSize = dataSize + sizeof(PacketHeader);

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
	header->size = packetSize;
	header->id = packetId;

	ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
	sendBuffer->SetWriteSizeWithDataSize(dataSize);

	return sendBuffer;
};