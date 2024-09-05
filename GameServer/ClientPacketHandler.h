#pragma once

enum : uint16
{
	PKT_S_SERVER_CHAT = 9999,

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

	PKT_C_MOVE = 1010,
	PKT_S_MOVE = 1011,

	PKT_C_SHOT = 1012,
	PKT_S_SHOT = 1013,

	PKT_C_HIT = 1014,
	PKT_S_HIT = 1015,
};

class ClientPacketHandler
{
public:
	static bool HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

	static bool HandleTest(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleSignup(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleLogin(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleEnterGame(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleChat(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleMove(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleShot(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
	static bool HandleHit(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
};

