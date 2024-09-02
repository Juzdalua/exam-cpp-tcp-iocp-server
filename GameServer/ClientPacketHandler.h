#pragma once

enum : uint16
{
	PKT_C_TEST = 0001,
	PKT_S_TEST = 0002,
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_ENTER_GAME = 1002,
	PKT_S_ENTER_GAME = 1003,
	PKT_C_CHAT = 1004,
	PKT_S_CHAT = 1005,
};

class ClientPacketHandler
{
public:
	static bool HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
};

