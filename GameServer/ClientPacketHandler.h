#pragma once

enum PKT_ID : uint16
{
	PKT_C_TEST = 1000,
	PKT_S_TEST = 1001,

	PKT_C_SIGNUP = 1002,
	PKT_S_SIGNUP = 1003,

	PKT_C_LOGIN = 1004,
	PKT_S_LOGIN = 1005,

	PKT_C_ENTER_GAME = 1006,
	PKT_S_ENTER_GAME = 1007,

	PKT_C_CHAT = 1008,
	PKT_S_CHAT = 1009,
};

class ClientPacketHandler
{
public:
	static bool HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	static bool HandleTest(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleSignup(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleLogin(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
};

